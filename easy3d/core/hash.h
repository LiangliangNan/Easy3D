/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_HASH_H
#define EASY3D_HASH_H

#include <functional>


namespace easy3d
{

    inline void hash_combine(std::size_t &seed, std::size_t hash) {
        hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash;
    }


    template <int DIM, typename FT> inline
    std::size_t hash(const Vec<DIM, FT>& v) {
        std::size_t seed = 0;
        std::hash<FT> hasher;
        for (std::size_t i=0; i<v.size(); ++i)
            hash_combine(seed, hasher(v[i]));
        return seed;
    }


    template<typename FT, typename InputIterator> inline
    std::size_t hash(InputIterator begin, InputIterator end) {
        std::size_t seed = 0;
        std::hash<FT> hasher;
        for (auto it = begin; it != end; ++it)
            hash_combine(seed, hasher(*it));
        return seed;
    }

} // namespace easy3d

#endif  // EASY3D_HASH_H