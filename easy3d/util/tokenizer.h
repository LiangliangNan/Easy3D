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

#include <string>
#include <cstdlib>

#define NV_MAX_TOKEN_LEN  1024
#define NV_MAX_DELIM_COUNT  16

namespace easy3d {

	/**
	 * \brief A utility class for splitting strings into tokens based on delimiters.
	 * \details This class provides methods to tokenize a given string based on a set of delimiters.
	 *		It can handle empty tokens and allows for retrieving individual tokens by index.
	 * \class Tokenizer easy3d/util/tokenizer.h
	 */
	class Tokenizer
	{
	public:
		/**
		 * \brief Constructor that initializes the tokenizer with a source string and optional delimiters.
		 * \param src The source string to tokenize.
		 * \param delims The delimiters to use for tokenizing. If nullptr, default delimiters are used.
		 */
		explicit Tokenizer(const char* src, const char* delims = nullptr)
			: mSrcBuf(src)
			, mTokLen(0)
			, mTermChar(0)
			, mNumDelims(0)
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

		/**
		 * \brief Sets whether to consume whitespace characters.
		 * \param ws True to consume whitespace, false otherwise.
		 */
		void setConsumeWS(bool ws) { mConsumeWS = ws; }

		/**
		 * \brief Checks if the given character is a whitespace character.
		 * \param c The character to check.
		 * \return True if the character is a whitespace character, false otherwise.
		 */
		bool isWhitespace(const char c) {
			return (' ' == c || '\t' == c);
		}
		/**
		 * \brief Checks if the given character is a quote character.
		 * \param c The character to check.
		 * \return True if the character is a quote character, false otherwise.
		 */
		bool isQuote(const char c) {
			return ('"' == c || '\'' == c);
		}
		/**
		 * \brief Checks if the given character is an end-of-line character.
		 * \param c The character to check.
		 * \return True if the character is an end-of-line character, false otherwise.
		 */
		bool isEOL(const char c) {
			return ('\n' == c || '\r' == c);
		}
		/**
		 * \brief Checks if the given character is a termination character.
		 * \param c The character to check.
		 * \return True if the character is a termination character, false otherwise.
		 */
		bool isTerm(const char c) {
			return (isWhitespace(c) || isEOL(c));
		}
		/**
		 * \brief Checks if the given character is a delimiter.
		 * \param c The character to check.
		 * \return True if the character is a delimiter, false otherwise.
		 */
		bool isDelim(const char c) { // TBD any other delims we want to call out.
			for (uint32_t i = 0; i < mNumDelims; i++)
				if (c == mDelims[i]) return true;
			return false;
		}


	public:
		/**
		 * \brief Checks if the tokenizer has reached the end of the source string.
		 * \return True if the tokenizer has reached the end of the source string, false otherwise.
		 */
		bool atEOF() {
			// EOF is if we've reached a null character. :)
			return (0 == *mSrcBuf);
		}
		/**
		 * \brief Consumes whitespace characters from the source string.
		 * \return The last whitespace character consumed.
		 */
		char consumeWhitespace()
		{
			//VERBOSE_TOKEN_DEBUG("consuming whitespace");
			mTermChar = 0;
			while (!atEOF() && isWhitespace(*mSrcBuf))
				mTermChar = *mSrcBuf++;
			return mTermChar;
		}
		/**
		 * \brief Consumes one delimiter character from the source string.
		 * \return The delimiter character consumed.
		 */
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
		/**
		 * \brief Consumes characters up to the end of the line.
		 */
		void consumeToEOL()
		{
			while (!atEOF() && !isEOL(*mSrcBuf)) // eat up to the EOL
				mSrcBuf++;
			while (!atEOF() && isEOL(*mSrcBuf)) // if not null, then eat EOL chars until gone, in case of /r/n type stuff...
				mSrcBuf++;
		}
		/**
		 * \brief Reads the next token from the source string.
		 * \return True if a token was successfully read, false otherwise.
		 */
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
		/**
		 * \brief Checks if the next token matches the given string.
		 * \param find The string to match.
		 * \return True if the next token matches the given string, false otherwise.
		 */
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
		/**
		 * \brief Checks if the next token matches the given string and consumes a delimiter.
		 * \param find The string to match.
		 * \return True if the next token matches the given string and a delimiter was consumed, false otherwise.
		 */
		bool requireTokenDelim(const char *find)
		{
			if (!requireToken(find))
				return false;
			if (!consumeOneDelim())
				return false;
			// accepted.
			return true;
		}

		/**
		 * \brief Gets the character that caused the stop of the last token read.
		 * \return The termination character.
		 */
		char getTermChar() const {
			return mTermChar;
		}

		/**
		 * \brief Gets the last read token as a std::string.
		 * \param returnTok The string to store the last read token.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
		bool getLastToken(std::string& returnTok)
		{
			// we just return what's already in the buffer.
			// handy for failure or unexpected cases.
			returnTok.assign(mTokBuf);
			return true;
		}

		/**
		 * \brief Gets the last read token as a const char*.
		 * \return The last read token.
		 */
		const char* getLastTokenPtr() {
			return mTokBuf;
		}

		/**
		 * \brief Gets the length of the last read token.
		 * \return The length of the last read token.
		 */
		uint32_t getLastTokenLen() const {
			return mTokLen;
		}

		/**
		 * \brief Gets the next token as a std::string.
		 * \param returnTok The string to store the next token.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
		bool getTokenString(std::string& returnTok)
		{
			if (!readToken()) {
				returnTok.clear();
				return false;
			}
			returnTok.assign(mTokBuf);
			return true;
		}

		/**
		 * \brief Gets the next token as a char array with a maximum size.
		 * \param out The char array to store the next token.
		 * \param outmax The maximum size of the char array.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
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

		/**
		 * \brief Gets the next token as a floating-point number.
		 * \param out The floating-point number to store the next token.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
		bool getTokenFloat(float &out)
		{
			if (!readToken()) {
				out = 0;
				return false;
			}
			out = (float)strtod(mTokBuf, nullptr);
			return true;
		}

		/**
		 * \brief Gets the next tokens as an array of floating-point numbers.
		 * \param out The array to store the floating-point numbers.
		 * \param size The size of the array.
		 * \return The number of elements read.
		 */
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

		/**
		 * \brief Gets the next tokens as an array of integer numbers.
		 * \param out The array to store the integer numbers.
		 * \param size The size of the array.
		 * \return The number of elements read.
		 */
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

		/**
		 * \brief Gets the next token as an integer.
		 * \param out The integer to store the next token.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
		bool getTokenInt(int32_t &out)
		{
			if (!readToken()) {
				out = 0;
				return false;
			}
			out = (int32_t)strtol(mTokBuf, nullptr, 0);
			return true;
		}

		/**
		 * \brief Gets the next token as an unsigned integer.
		 * \param out The unsigned integer to store the next token.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
		bool getTokenUint(uint32_t &out)
		{
			if (!readToken()) {
				out = 0;
				return false;
			}
			out = (uint32_t)strtoul(mTokBuf, nullptr, 0);
			return true;
		}

		/**
		 * \brief Gets the next token as a boolean value.
		 * \param out The boolean value to store the next token.
		 * \return True if the token was successfully retrieved, false otherwise.
		 */
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

	protected:
		const char* mSrcBuf; /*< This holds a pointer to an immutable char buffer. */
		char mTokBuf[NV_MAX_TOKEN_LEN]; /*< This is a temporary buffer for collecting next token. */
		uint32_t mTokLen; /*< This is the character length of current token, not including null termination. */
		char mTermChar; /*< This is the character that caused us to stop parsing the current token. */
		char mDelims[NV_MAX_DELIM_COUNT];
		uint32_t mNumDelims;
	private:
		bool mConsumeWS;
	};

}

#endif
