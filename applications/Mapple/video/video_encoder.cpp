/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include "video_encoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "libavutil/imgutils.h"
}

#include <easy3d/util/logging.h>

namespace internal {

	// a wrapper around a single output AVStream
	typedef struct OutputStream {
		AVStream *st;
		AVCodecContext *enc;

		/* pts of the next frame that will be generated */
		int64_t next_pts;
		AVFrame *frame;
		AVPacket *tmp_pkt;

		struct SwsContext *sws_ctx;
	} OutputStream;


	static std::string av_error_string(int errnum) {
		char buffer[AV_ERROR_MAX_STRING_SIZE] = { 0 };
		return av_make_error_string(buffer, AV_ERROR_MAX_STRING_SIZE, errnum);
	}


	/* Add an output stream. */
	static void add_stream(OutputStream *ost, AVFormatContext *oc,
		const AVCodec **codec, enum AVCodecID codec_id, int width, int height, int framerate, int bitrate)
	{
		AVCodecContext *c;

		/* find the encoder */
		*codec = avcodec_find_encoder(codec_id);
		if (!(*codec)) {
			fprintf(stderr, "Could not find encoder for '%s'\n",
				avcodec_get_name(codec_id));
			exit(1);
		}

		ost->tmp_pkt = av_packet_alloc();
		if (!ost->tmp_pkt) {
			fprintf(stderr, "Could not allocate AVPacket\n");
			exit(1);
		}

		ost->st = avformat_new_stream(oc, NULL);
		if (!ost->st) {
			fprintf(stderr, "Could not allocate stream\n");
			exit(1);
		}
		ost->st->id = oc->nb_streams - 1;
		c = avcodec_alloc_context3(*codec);
		if (!c) {
			fprintf(stderr, "Could not alloc an encoding context\n");
			exit(1);
		}
		ost->enc = c;

		switch ((*codec)->type) {
		case AVMEDIA_TYPE_AUDIO:
			break;
		case AVMEDIA_TYPE_VIDEO:
			c->codec_id = codec_id;

			c->bit_rate = bitrate;
			/* Resolution must be a multiple of two. */
			c->width = width;
			c->height = height;
			/* timebase: This is the fundamental unit of time (in seconds) in terms
			 * of which frame timestamps are represented. For fixed-fps content,
			 * timebase should be 1/framerate and timestamp increments should be
			 * identical to 1. */
			ost->st->time_base = AVRational{ 1, framerate };
			c->time_base = ost->st->time_base;

			c->gop_size = 12; /* emit one intra frame every twelve frames at most */
			c->pix_fmt = AV_PIX_FMT_YUV420P;
			if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
				/* just for testing, we also add B-frames */
				c->max_b_frames = 2;
			}
			if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
				/* Needed to avoid using macroblocks in which some coeffs overflow.
				 * This does not happen with normal video, it just happens here as
				 * the motion of the chroma plane does not match the luma plane. */
				c->mb_decision = 2;
			}
			break;

		default:
			break;
		}

		/* Some formats want stream headers to be separate. */
		if (oc->oformat->flags & AVFMT_GLOBALHEADER)
			c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}


	/**************************************************************/
	/* video output */

	static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
	{
		AVFrame *picture;
		int ret;

		picture = av_frame_alloc();
		if (!picture)
			return NULL;

		picture->format = pix_fmt;
		picture->width = width;
		picture->height = height;

		/* allocate the buffers for the frame data */
		ret = av_frame_get_buffer(picture, 0);
		if (ret < 0) {
			fprintf(stderr, "Could not allocate frame data.\n");
			exit(1);
		}

		return picture;
	}

	static void open_video(AVFormatContext *oc, const AVCodec *codec,
		OutputStream *ost, AVDictionary *opt_arg)
	{
		int ret;
		AVCodecContext *c = ost->enc;
		AVDictionary *opt = NULL;

		av_dict_copy(&opt, opt_arg, 0);

		/* open the codec */
		ret = avcodec_open2(c, codec, &opt);
		av_dict_free(&opt);
		if (ret < 0) {
			//		fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
			exit(1);
		}

		/* allocate and init a re-usable frame */
		ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
		if (!ost->frame) {
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}

		/* copy the stream parameters to the muxer */
		ret = avcodec_parameters_from_context(ost->st->codecpar, c);
		if (ret < 0) {
			fprintf(stderr, "Could not copy the stream parameters\n");
			exit(1);
		}
	}


	static AVFrame *get_video_frame(OutputStream *ost, const unsigned char* image_data, int width, int height, int channels)
	{
		AVCodecContext *c = ost->enc;

		// 	/* check if we want to generate more frames */
		// 	if (av_compare_ts(ost->next_pts, c->time_base,
		// 		STREAM_DURATION, AVRational{ 1, 1 }) > 0)
		// 		return NULL;

			/* when we pass a frame to the encoder, it may keep a reference to it
			 * internally; make sure we do not overwrite it here */
		if (av_frame_make_writable(ost->frame) < 0)
			exit(1);

		/* as we only generate a YUV420P picture, we must convert it
		 * to the codec pixel format if needed */

		if (!ost->sws_ctx) {
			ost->sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGRA, width, height, c->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
			if (!ost->sws_ctx) {
				fprintf(stderr,
					"could not initialize the conversion context\n");
				exit(1);
			}
		}

		int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, width, height, 1);
		if (num_bytes != width * height * channels) {
			std::string error_msg = "number of bytes mismatch";
			LOG(ERROR) << error_msg;
			throw std::runtime_error(error_msg);
		}

		const uint8_t *srcSlice[3] = { image_data, 0, 0 };
		int srcStride[3] = { width * channels, 0, 0 }; // first element is bytes per line

		sws_scale(ost->sws_ctx, srcSlice, srcStride, 0, height, ost->frame->data, ost->frame->linesize);

		ost->frame->pts = ost->next_pts++;

		return ost->frame;
	}


	static void close_stream(AVFormatContext *oc, OutputStream *ost)
	{
		avcodec_free_context(&ost->enc);
		av_frame_free(&ost->frame);
		av_packet_free(&ost->tmp_pkt);
		sws_freeContext(ost->sws_ctx);
	}


	class VideoEncoderImpl {
	public:
		VideoEncoderImpl() : width_(0), height_(0), channels_(0) {}

		bool start(const std::string& filename, int framerate, int bitrate);
		bool write_frame(const unsigned char* data, int width, int height, int channels);
		void end();

	public:
		OutputStream video_st = { 0 };
		const AVOutputFormat *fmt;
		AVFormatContext *fmt_ctx;
		const AVCodec *video_codec = nullptr;
		AVDictionary *opt = nullptr;
		int framerate_;
        int bitrate_;
		int width_;
		int height_;
		int channels_;
		std::string filename_;
	};


	bool VideoEncoderImpl::start(const std::string& filename, int framerate, int bitrate)
	{
		if (filename.empty()) {
			LOG(ERROR) << "file name of the output video is empty";
            return false;
		}
		filename_ = filename;
		framerate_ = framerate;
        bitrate_ = bitrate;

		/* allocate the output media context */
		avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, filename_.c_str());
		if (!fmt_ctx) {
			printf("Could not deduce output format from file extension: using MPEG.\n");
			avformat_alloc_output_context2(&fmt_ctx, NULL, "mpeg", filename_.c_str());
		}
		if (!fmt_ctx)
			return false;

		fmt = fmt_ctx->oformat;
		return true;
	}


	bool VideoEncoderImpl::write_frame(const unsigned char* data, int width, int height, int channels) {
		if (width <= 0 || height <= 0 || channels < 3 || channels > 4) {
			std::string error_msg = "frame size not accepted: width=" + std::to_string(width) + ", height=" + std::to_string(height) + ", channels=" + std::to_string(channels);
			LOG(ERROR) << error_msg;
			throw std::runtime_error(error_msg);
		}

		// not initialized
		if (width_ == 0 || height_ == 0 || channels_ == 0) {
			width_ = width;
			height_ = height;
			channels_ = channels;

			/* Add the video stream using the default format codecs
			 * and initialize the codecs. */
			if (fmt->video_codec != AV_CODEC_ID_NONE) {
				add_stream(&video_st, fmt_ctx, &video_codec, fmt->video_codec, width_, height_, framerate_, bitrate_);
			}

			/* Now that all the parameters are set, we can open the video codec
			 * and allocate the necessary encode buffers. */
			open_video(fmt_ctx, video_codec, &video_st, opt);

			av_dump_format(fmt_ctx, 0, filename_.c_str(), 1);

			/* open the output file, if needed */
			if (!(fmt->flags & AVFMT_NOFILE)) {
				int ret = avio_open(&fmt_ctx->pb, filename_.c_str(), AVIO_FLAG_WRITE);
				if (ret < 0) {
					std::string error_msg = "could not open " + filename_ + ". " + av_error_string(ret);
					LOG(ERROR) << error_msg;
					throw std::runtime_error(error_msg);
				}
			}

			/* Write the stream header, if any. */
			int ret = avformat_write_header(fmt_ctx, &opt);
			if (ret < 0) {
				std::string error_msg = "error occurred when opening output file: " + av_error_string(ret);
				LOG(ERROR) << error_msg;
				throw std::runtime_error(error_msg);
			}
		}

		AVFrame* frame = get_video_frame(&video_st, data, width, height, channels);

		// send the frame to the encoder
		int ret = avcodec_send_frame(video_st.enc, frame);
		if (ret < 0) {
			std::string error_msg = "error sending a frame to the encoder: " + av_error_string(ret);
			LOG(ERROR) << error_msg;
			throw std::runtime_error(error_msg);
		}

		while (ret >= 0) {
			ret = avcodec_receive_packet(video_st.enc, video_st.tmp_pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			else if (ret < 0) {
				std::string error_msg = "error encoding a frame: " + av_error_string(ret);
				LOG(ERROR) << error_msg;
				throw std::runtime_error(error_msg);
			}

			/* rescale output packet timestamp values from codec to stream timebase */
			av_packet_rescale_ts(video_st.tmp_pkt, video_st.enc->time_base, video_st.st->time_base);
			video_st.tmp_pkt->stream_index = video_st.st->index;

			/* Write the compressed frame to the media file. */
			ret = av_interleaved_write_frame(fmt_ctx, video_st.tmp_pkt);
			/* pkt is now blank (av_interleaved_write_frame() takes ownership of
			 * its contents and resets pkt), so that no unreferencing is necessary.
			 * This would be different if one used av_write_frame(). */
			if (ret < 0) {
				std::string error_msg = "error while writing output packet: " + av_error_string(ret);
				LOG(ERROR) << error_msg;
				throw std::runtime_error(error_msg);
			}
		}

		return (ret == AVERROR_EOF) ? false : true;
	}


	void VideoEncoderImpl::end() {
		av_write_trailer(fmt_ctx);

		/* Close each codec. */
		close_stream(fmt_ctx, &video_st);

		if (!(fmt->flags & AVFMT_NOFILE))
			/* Close the output file. */
			avio_closep(&fmt_ctx->pb);

		/* free the stream */
		avformat_free_context(fmt_ctx);
	}

}


using namespace internal;

namespace easy3d {

	VideoEncoder::VideoEncoder() : encoder_(nullptr) {
#ifdef NDEBUG
		av_log_set_level(AV_LOG_QUIET);
#else
		av_log_set_level(AV_LOG_INFO);
#endif
	}


	VideoEncoder::~VideoEncoder() {
		delete encoder_;
	}


	bool VideoEncoder::start(const std::string& filename, int framerate, int bitrate)
	{
		if (!encoder_)
			encoder_ = new VideoEncoderImpl;

		return encoder_->start(filename, framerate, bitrate);
	}



	bool VideoEncoder::encode_frame(const unsigned char* data, int width, int height, int channels) {
		if (!encoder_) {
			std::string error_msg = "the video encoder has not started yet";
			LOG(ERROR) << error_msg;
			throw std::runtime_error(error_msg);
		}

		if (encoder_->width_ != 0 && encoder_->height_ != 0 && encoder_->channels_ != 0) { // already initialized with the image dimension data
			if (width != encoder_->width_ || height != encoder_->height_ || channels != encoder_->channels_) {
				std::string error_msg = "image size differs from the size of the previously created video stream";
				LOG(ERROR) << error_msg;
				throw std::runtime_error(error_msg);
			}
		}

		return encoder_->write_frame(data, width, height, channels);
	}


	bool VideoEncoder::end() {
		if (!encoder_) {
			std::string error_msg = "the video encoder has not started yet";
			LOG(ERROR) << error_msg;
			throw std::runtime_error(error_msg);
		}

		encoder_->end();

		delete encoder_;
		encoder_ = nullptr;

		return true;
	}

}