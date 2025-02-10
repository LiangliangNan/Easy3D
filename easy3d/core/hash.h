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

#ifndef EASY3D_CORE_HASH_H
#define EASY3D_CORE_HASH_H


#include <cstdint>
#include <functional>

#include <easy3d/core/vec.h>


namespace easy3d
{
    /**
     * \brief Combines a hash value with a seed value.
     * \details std::size_t has 64 bits on most systems, but 32 bits on 32-bit Windows. To make the same code robustly
     *     run on both 32-bit and 64-bit systems, Easy3D uses 64-bit integer for hash values.
     *     This function implements the 64-bit hash combine algorithm (inspired by the \c Hash128to64 function in
     *     [CityHash](https://github.com/google/cityhash/blob/master/src/city.h)).
     * \tparam T The type of the value to hash.
     * \param seed The seed value to combine with.
     * \param value The value to hash.
     */
    template<typename T>
    void hash_combine(uint64_t &seed, T const& value) {
        static std::hash<T> hasher;
        uint64_t a = (hasher(value) ^ seed) * 0x9ddfea08eb382d69ULL;
        a ^= (a >> 47);
        uint64_t b = (seed ^ a) * 0x9ddfea08eb382d69ULL;
        b ^= (b >> 47);
        seed = b * 0x9ddfea08eb382d69ULL;
    }

    /**
     * \brief Computes the hash value of a 2D vector.
     * \tparam FT The floating point type.
     * \param value The 2D vector to hash.
     * \return The computed hash value.
     */
    template <typename FT>
    uint64_t hash(const Vec<2, FT>& value) {
        uint64_t seed(0);
        hash_combine(seed, value.x);
        hash_combine(seed, value.y);
        return seed;
    }

    /**
     * \brief Computes the hash value of a 3D vector.
     * \tparam FT The floating point type.
     * \param value The 3D vector to hash.
     * \return The computed hash value.
     */
    template <typename FT>
    uint64_t hash(const Vec<3, FT>& value) {
        uint64_t seed(0);
        hash_combine(seed, value.x);
        hash_combine(seed, value.y);
        hash_combine(seed, value.z);
        return seed;
    }

    /**
     * \brief Computes the hash value of a vector with a given dimension.
     * \tparam DIM The dimension of the vector.
     * \tparam FT The floating point type.
     * \param value The vector to hash.
     * \return The computed hash value.
     */
    template <int DIM, typename FT>
    uint64_t hash(const Vec<DIM, FT>& value) {
        uint64_t seed(0);
        for (int i=0; i<DIM; ++i)
            hash_combine(seed, value[i]);
        return seed;
    }

    /**
     * \brief Computes the hash value of a 1D array.
     * \tparam Iterator The type of the iterator.
     * \param first The iterator pointing to the first element.
     * \param last The iterator pointing to the past-the-end element.
     * \return The computed hash value.
     */
    template<typename Iterator>
    uint64_t hash(Iterator first, Iterator last) {
        uint64_t seed(0);
        for (; first != last; ++first) {
            hash_combine(seed, *first);
        }
        return seed;
    }

    /**
     * \brief Computes the hash value of a 1D array with a given seed value.
     * \tparam Iterator The type of the iterator.
     * \param seed The seed value to combine with.
     * \param first The iterator pointing to the first element.
     * \param last The iterator pointing to the past-the-end element.
     */
    template<typename Iterator>
    void hash(uint64_t &seed, Iterator first, Iterator last) {
        for (; first != last; ++first) {
            hash_combine(seed, *first);
        }
    }

} // namespace easy3d

#endif  // EASY3D_CORE_HASH_H