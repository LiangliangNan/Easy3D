//----------------------------------------------------------------------------------
// File:        NV/NvTokenizer.h
// SDK Version: v3.00
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014-2015, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------

#ifndef NV_TOKENIZER_H
#define NV_TOKENIZER_H

//#include <NvSimpleTypes.h>
#include <string>
#include <cstdlib>

/// \file
/// Classes for tokenizing of input character streams.

#define NV_MAX_TOKEN_LEN  1024
#define NV_MAX_DELIM_COUNT  16

namespace easy3d {


	class Tokenizer
	{
	protected:
		const char* mSrcBuf; /*< This holds a pointer to an immutable char buffer. */
		char mTokBuf[NV_MAX_TOKEN_LEN]; /*< This is a temporary buffer for collecting next token. */
		uint32_t mTokLen; /*< This is the character length of current token, not including null termination. */
		char mTermChar; /*< This is the character that caused us to stop parsing the current token. */
		char mDelims[NV_MAX_DELIM_COUNT];
		uint32_t mNumDelims;
	private:
		bool mVerbose;
		bool mConsumeWS;

	public:
		Tokenizer(const char* src, const char* delims = nullptr)
			: mSrcBuf(src)
			, mTokLen(0)
			, mTermChar(0)
			, mNumDelims(0)
			, mVerbose(false)
			, mConsumeWS(true)
		{
			if (nullptr == delims)
			{ // built in delim set.
				mNumDelims = 3;
				mDelims[0] = '=';
				mDelims[1] = ',';
				mDelims[2] = ':';
			}
			else
			{   // note that we DO support empty string, in which case
				// we have no delims other than whitespace...
				int i = 0;
				mNumDelims = 0;
				while (mNumDelims < NV_MAX_DELIM_COUNT && delims[i])
				{
					const char c = delims[i++];
					// else skip built-in handled chars
					if (c == ' ' || c == '\t' || // Whitespace
						c == '"' || c == '\'' || // Quote
						c == '\n' || c == '\r') // EOL
						continue;
					mDelims[mNumDelims++] = c;
				}
			}
		}

		void setVerbose() { mVerbose = true; }
		void setConsumeWS(bool ws) { mConsumeWS = ws; }

		inline bool isWhitespace(const char c)
		{
			return (' ' == c || '\t' == c);
		}

		inline bool isQuote(const char c)
		{
			return ('"' == c || '\'' == c);
		}

		inline bool isEOL(const char c)
		{
			return ('\n' == c || '\r' == c);
		}

		inline bool isTerm(const char c)
		{
			return (isWhitespace(c) || isEOL(c));
		}

		inline bool isDelim(const char c)
		{ // TBD any other delims we want to call out.
			for (uint32_t i = 0; i < mNumDelims; i++)
				if (c == mDelims[i]) return true;
			return false;
		}


	public:
		inline bool atEOF() {
			// EOF is if we've reached a null character. :)
			return (0 == *mSrcBuf);
		}

		char consumeWhitespace()
		{
			//VERBOSE_TOKEN_DEBUG("consuming whitespace");
			mTermChar = 0;
			while (!atEOF() && isWhitespace(*mSrcBuf))
				mTermChar = *mSrcBuf++;
			return mTermChar;
		}

		char consumeOneDelim()
		{
			//VERBOSE_TOKEN_DEBUG("consuming delimiter");
			consumeWhitespace();
			mTermChar = 0;
			// eat ONE delimiter...
			if (!atEOF() && isDelim(*mSrcBuf)) {
				mTermChar = *mSrcBuf++;
			}
			if (isEOL(*mSrcBuf))
				mTermChar = *mSrcBuf; // so that we return the EOL but DON'T CONSUME IT.
			return mTermChar;
		}

		void consumeToEOL()
		{
			while (!atEOF() && !isEOL(*mSrcBuf)) // eat up to the EOL
				mSrcBuf++;
			while (!atEOF() && isEOL(*mSrcBuf)) // if not null, then eat EOL chars until gone, in case of /r/n type stuff...
				mSrcBuf++;
		}

		bool readToken()
		{
			char startedWithQuote = 0; // we'll store the character if we get a quote
			mTermChar = 0;
			mTokLen = 0;
			mTokBuf[0] = 0; // initialize
			if (atEOF())
				return false; // exit early...

			if (mConsumeWS)
				consumeWhitespace();
			if (!atEOF() && isQuote(*mSrcBuf)) {
				startedWithQuote = *mSrcBuf; // save WHAT quote character so we match it.
				mSrcBuf++;
			}

			while (!atEOF()) { // termchar is already null so fine to exit loop.
				// look for hard terminations (EOL)
				if (isEOL(*mSrcBuf)) {
					mTermChar = *mSrcBuf;
					break;
				}
				// look to handle quoted strings, which CAN have delimiters validly in their contents.
				if (startedWithQuote) {
					if (startedWithQuote == *mSrcBuf) {
						// consume and break.
						mSrcBuf++;
						mTermChar = startedWithQuote;
						break;
					}
				}
				else if (isDelim(*mSrcBuf) || isWhitespace(*mSrcBuf)) {
					// just break, leave delim.
					mTermChar = *mSrcBuf;
					break;
				}

				mTokBuf[mTokLen] = *mSrcBuf; // copy char
				mTokLen++; // inc length
				mSrcBuf++; // inc buffer
			}

			// null-term buffer, do NOT inc length.
			mTokBuf[mTokLen] = 0;

			return (mTokLen > 0 || startedWithQuote); // false if empty string UNLESS quoted empty string...
		}

		bool requireToken(const char *find)
		{
			if (find == nullptr || find[0] == 0)
				return false; // bad input.
			if (!readToken())
				return false;
			// let's eliminate str functions...
			const size_t findlen = strlen(find);
			if (findlen != mTokLen)
				return false; // early out.
			if (0 != memcmp(mTokBuf, find, findlen))
				return false;
			// accepted.
			return true;
		}

		bool requireTokenDelim(const char *find)
		{
			if (!requireToken(find))
				return false;
			if (!consumeOneDelim())
				return false;
			// accepted.
			return true;
		}

		/// accessor to get character that caused 'stop' of last token read
		char getTermChar() {
			return mTermChar;
		}

		/// get last read token as a std::string
		bool getLastToken(std::string& returnTok)
		{
			// we just return what's already in the buffer.
			// handy for failure or unexpected cases.
			returnTok.assign(mTokBuf);
			return true;
		}

		/// accessor to get last read token const char *
		const char* getLastTokenPtr()
		{
			return mTokBuf;
		}

		/// accessor to get last read token length
		uint32_t getLastTokenLen()
		{
			return mTokLen;
		}


		/// get next token as a std::string
		bool getTokenString(std::string& returnTok)
		{
			if (!readToken()) {
				returnTok.clear();
				return false;
			}
			returnTok.assign(mTokBuf);
			return true;
		}

		/// get next token as a char array with maximum size.
		bool getTokenString(char out[], const uint32_t outmax)
		{
			if (!readToken()) {
				out[0] = 0;
				return false;
			}

			// note mTokLen include the NULL terminator.
			if (mTokLen > outmax)
			{
				// just have to truncate.
				memcpy(out, mTokBuf, outmax - 1);
				out[outmax - 1] = 0; // null terminate...
			}
			else
				memcpy(out, mTokBuf, mTokLen + 1); // copy null!
			return true;
		}

		/// get next token as a floating-point number
		bool getTokenFloat(float &out)
		{
			if (!readToken()) {
				out = 0;
				return false;
			}
			out = (float)strtod(mTokBuf, nullptr);
			return true;
		}

		/// get next tokens as array of floating point numbers
		uint32_t getTokenFloatArray(float out[], uint32_t size)
		{
			uint32_t i = 0;
			char firstdelim = 0, delim = 0;
			while (i < size) {
				if (firstdelim) {
					// we had a delim initially, so we require (but can have repeated).
					delim = consumeOneDelim();
					if (delim == firstdelim)
						continue;
				}
				if (!readToken())
					break; // so we return what we've got.
				out[i++] = (float)strtod(mTokBuf, nullptr);

				// OPTIONALLY consume a delimiter between each number.
				delim = consumeOneDelim();
				if (delim && isEOL(delim)) // we didn't consume EOL, we are AT it though.
					break;
				if (i == 1) firstdelim = delim; // stash away.
			}
			return i; // return number of elements read.
		}

		/// get next tokens as array of integer numbers
		uint32_t getTokenIntArray(int32_t out[], uint32_t size)
		{
			uint32_t i = 0;
			char firstdelim = 0, delim = 0;
			while (i < size) {
				if (firstdelim) {
					// we had a delim initially, so we require (but can have repeated).
					delim = consumeOneDelim();
					if (delim == firstdelim)
						continue;
				}
				if (!readToken())
					break; // so we return what we've got.
				out[i++] = (int32_t)strtol(mTokBuf, nullptr, 0);

				// OPTIONALLY consume a delimiter between each number.
				delim = consumeOneDelim();
				if (delim && isEOL(delim)) // we didn't consume EOL, we are AT it though.
					break;
				if (i == 1) firstdelim = delim; // stash away.
			}
			return i; // return number of elements read.
		}

		/// get next token as an integer
		bool getTokenInt(int32_t &out)
		{
			if (!readToken()) {
				out = 0;
				return false;
			}
			out = (int32_t)strtol(mTokBuf, nullptr, 0);
			return true;
		}

		/// get next token as an unsigned integer
		bool getTokenUint(uint32_t &out)
		{
			if (!readToken()) {
				out = 0;
				return false;
			}
			out = (uint32_t)strtoul(mTokBuf, nullptr, 0);
			return true;
		}

		/// get next token as some form of boolean value/string
		bool getTokenBool(bool &out)
		{
			if (!readToken()) {
				out = false;
				return false;
			}
			if (mTokLen == 1 &&
				(mTokBuf[0] == '0' || mTokBuf[0] == '1')) {
				out = (mTokBuf[0] == '1');
				return true;
			}
			else if ((0 == strcmp(mTokBuf, "true")) ||
				(0 == strcmp(mTokBuf, "TRUE")) ||
				(0 == strcmp(mTokBuf, "yes")) ||
				(0 == strcmp(mTokBuf, "YES"))) {
				out = true;
				return true;
			}
			else if ((0 == strcmp(mTokBuf, "false")) ||
				(0 == strcmp(mTokBuf, "FALSE")) ||
				(0 == strcmp(mTokBuf, "no")) ||
				(0 == strcmp(mTokBuf, "NO"))) {
				out = false;
				return true;
			}
			// ... otherwise, no boolean value detected.
			return false;
		}
	};

}

#endif
