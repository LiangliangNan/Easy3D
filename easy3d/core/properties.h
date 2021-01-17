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

/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications and
 * significant enhancement.
 *		- Surface_mesh (version 1.1)
 * The original code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *
 *----------------------------------------------------------*/

#ifndef EASY3D_CORE_PROPERTIES_H
#define EASY3D_CORE_PROPERTIES_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <cassert>


namespace easy3d {

    /// \brief Base class for a property array.
    /// \class BasePropertyArray easy3d/core/properties.h
    class BasePropertyArray
    {
    public:

        /// Default constructor
        BasePropertyArray(const std::string& name) : name_(name) {}

        /// Destructor.
        virtual ~BasePropertyArray() {}

        /// Reserve memory for n elements.
        virtual void reserve(size_t n) = 0;

        /// Resize storage to hold n elements.
        virtual void resize(size_t n) = 0;

        /// Free unused memory.
        virtual void shrink_to_fit() = 0;

        /// Extend the number of elements by one.
        virtual void push_back() = 0;

        /// Reset element to default value
        virtual void reset(size_t idx) = 0;

        virtual bool transfer(const BasePropertyArray& other) = 0;
        virtual bool transfer(const BasePropertyArray& other, std::size_t from, std::size_t to) = 0;

        /// Let two elements swap their storage place.
        virtual void swap(size_t i0, size_t i1) = 0;

        /// Let copy 'from' -> 'to'.
        virtual void copy(size_t from, size_t to) = 0;

        /// Return a deep copy of self.
        virtual BasePropertyArray* clone () const = 0;

        /// Return a empty copy of self.
        virtual BasePropertyArray* empty_clone () const = 0;

        /// Return the type_info of the property
        virtual const std::type_info& type() const = 0;

        /// Return the name of the property
        const std::string& name() const { return name_; }

        /// Set the name of the property
        void set_name(const std::string& n) { name_ = n; }

        bool is_same (const BasePropertyArray& other)
        {
            return (name() == other.name() && type() == other.type());
        }

    protected:

        std::string name_;
    };



    //== CLASS DEFINITION =========================================================

    /// \brief Implementation of a generic property array.
    /// \class PropertyArray easy3d/core/properties.h
    template <class T>
    class PropertyArray : public BasePropertyArray
    {
    public:

        typedef T                                       value_type;
        typedef std::vector<value_type>                 vector_type;
        typedef typename vector_type::reference         reference;
        typedef typename vector_type::const_reference   const_reference;

        PropertyArray(const std::string& name, T t=T()) : BasePropertyArray(name), value_(t) {}


    public: // virtual interface of BasePropertyArray

        virtual void reserve(size_t n)
        {
            data_.reserve(n);
        }

        virtual void resize(size_t n)
        {
            data_.resize(n, value_);
        }

        virtual void push_back()
        {
            data_.push_back(value_);
        }

        virtual void reset(size_t idx)
        {
            data_[idx] = value_;
        }

        bool transfer(const BasePropertyArray& other)
        {
            const PropertyArray<T>* pa = dynamic_cast<const PropertyArray*>(&other);
            if(pa != nullptr){
                std::copy((*pa).data_.begin(), (*pa).data_.end(), data_.end()-(*pa).data_.size());
                return true;
            }
            return false;
        }

        bool transfer(const BasePropertyArray& other, std::size_t from, std::size_t to)
        {
            const PropertyArray<T>* pa = dynamic_cast<const PropertyArray*>(&other);
            if (pa != nullptr)
            {
                data_[to] = (*pa)[from];
                return true;
            }

            return false;
        }

        virtual void shrink_to_fit()
        {
            vector_type(data_).swap(data_);
        }

        virtual void swap(size_t i0, size_t i1)
        {
            T d(data_[i0]);
            data_[i0]=data_[i1];
            data_[i1]=d;
        }

        virtual void copy(size_t from, size_t to)
        {
            data_[to]=data_[from];
        }

        virtual BasePropertyArray* clone() const
        {
            PropertyArray<T>* p = new PropertyArray<T>(name_, value_);
            p->data_ = data_;
            return p;
        }

        virtual BasePropertyArray* empty_clone() const
        {
            PropertyArray<T>* p = new PropertyArray<T>(this->name_, this->value_);
            return p;
        }

        virtual const std::type_info& type() const { return typeid(T); }


    public:

        /// Get pointer to array (does not work for T==bool)
        const T* data() const
        {
            return &data_[0];
        }


        /// Get reference to the underlying vector
        std::vector<T>& vector()
        {
            return data_;
        }


        /// Access the i'th element. No range check is performed!
        reference operator[](size_t _idx)
        {
            assert( size_t(_idx) < data_.size() );
            return data_[_idx];
        }

        /// Const access to the i'th element. No range check is performed!
        const_reference operator[](size_t _idx) const
        {
            assert( size_t(_idx) < data_.size());
            return data_[_idx];
        }


    private:
        vector_type data_;
        value_type  value_;
    };


    // specialization for bool properties
    template <>
    inline const bool*
    PropertyArray<bool>::data() const
    {
        assert(false);
        return nullptr;
    }



    //== CLASS DEFINITION =========================================================

    /// \brief Implementation of a generic property.
    /// \class Property easy3d/core/properties.h
    template <class T>
    class Property
    {
    public:

        typedef typename PropertyArray<T>::reference reference;
        typedef typename PropertyArray<T>::const_reference const_reference;

        friend class PropertyContainer;

    public:

        Property(PropertyArray<T> *p = nullptr) : parray_(p) {}

        void reset()
        {
            parray_ = nullptr;
        }

        operator bool() const
        {
            return parray_ != nullptr;
        }

        reference operator[](size_t i)
        {
            assert(parray_ != nullptr);
            return (*parray_)[i];
        }

        const_reference operator[](size_t i) const
        {
            assert(parray_ != nullptr);
            return (*parray_)[i];
        }

        const T* data() const
        {
            assert(parray_ != nullptr);
            return parray_->data();
        }

        std::vector<T>& vector()
        {
            assert(parray_ != nullptr);
            return parray_->vector();
        }

        const std::vector<T>& vector() const
        {
            assert(parray_ != nullptr);
            return parray_->vector();
        }

        PropertyArray<T>& array()
        {
            assert(parray_ != nullptr);
            return *parray_;
        }

        const PropertyArray<T>& array() const
        {
            assert(parray_ != nullptr);
            return *parray_;
        }

        /// Return the name of the property
        const std::string& name() const {
            assert(parray_ != nullptr);
            return parray_->name();
        }

        /// Set the name of the property
        void set_name(const std::string& n) {
            assert(parray_ != nullptr);
            parray_->set_name(n);
        }

    private:
        PropertyArray<T>* parray_;
    };



    //== CLASS DEFINITION =========================================================


    /// \brief Implementation of generic property container.
    /// \class PropertyContainer easy3d/core/properties.h
    class PropertyContainer
    {
    public:

        // default constructor
        PropertyContainer() : size_(0) {}

        // destructor (deletes all property arrays)
        virtual ~PropertyContainer() { clear(); }

        // copy constructor: performs deep copy of property arrays
        PropertyContainer(const PropertyContainer& _rhs) { operator=(_rhs); }

        // assignment: performs deep copy of property arrays
        PropertyContainer& operator=(const PropertyContainer& _rhs)
        {
            if (this != &_rhs)
            {
                clear();
                parrays_.resize(_rhs.n_properties());
                size_ = _rhs.size();
                for (size_t i=0; i<parrays_.size(); ++i)
                    parrays_[i] = _rhs.parrays_[i]->clone();
            }
            return *this;
        }

        void transfer(const PropertyContainer& _rhs)
        {
            for(std::size_t i=0; i<parrays_.size(); ++i){
                for (std::size_t j=0; j<_rhs.parrays_.size(); ++j){
                    if(parrays_[i]->is_same (*(_rhs.parrays_[j]))){
                        parrays_[i]->transfer(* _rhs.parrays_[j]);
                        break;
                    }
                }
            }
        }

        // Copy properties that don't already exist from another container
        void copy_properties (const PropertyContainer& _rhs)
        {
            for (std::size_t i = 0; i < _rhs.parrays_.size(); ++ i)
            {
                bool property_already_exists = false;
                for (std::size_t j = 0; j < parrays_.size(); ++ j)
                    if (_rhs.parrays_[i]->is_same (*(parrays_[j])))
                    {
                        property_already_exists = true;
                        break;
                    }

                if (property_already_exists)
                    continue;

                parrays_.push_back (_rhs.parrays_[i]->empty_clone());
                parrays_.back()->resize(size_);
            }
        }

        // Transfer one element with all properties
        // WARNING: properties must be the same in the two containers
        bool transfer(const PropertyContainer& _rhs, std::size_t from, std::size_t to)
        {
            bool out = true;
            for(std::size_t i=0; i<parrays_.size(); ++i)
                if (!(parrays_[i]->transfer(* _rhs.parrays_[i], from, to)))
                    out = false;
            return out;
        }

        // returns the current size of the property arrays
        size_t size() const { return size_; }

        // returns the number of property arrays
        size_t n_properties() const { return parrays_.size(); }

        // returns a vector of all property names
        std::vector<std::string> properties() const
        {
            std::vector<std::string> names;
            for (size_t i=0; i<parrays_.size(); ++i)
                names.push_back(parrays_[i]->name());
            return names;
        }


        // add a property with name \c name and default value \c t
        template <class T> Property<T> add(const std::string& name, const T t=T())
        {
            // if a property with this name already exists, return an invalid property
            for (size_t i=0; i<parrays_.size(); ++i)
            {
                if (parrays_[i]->name() == name)
                {
                    LOG(ERROR) << "A property with name \""
                              << name << "\" already exists. Returning invalid property.";
                    return Property<T>();
                }
            }

            // otherwise add the property
            PropertyArray<T>* p = new PropertyArray<T>(name, t);
            p->resize(size_);
            parrays_.push_back(p);
            return Property<T>(p);
        }


        // get a property by its name. returns invalid property if it does not exist.
        template <class T> Property<T> get(const std::string& name) const
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                if (parrays_[i]->name() == name)
                    return Property<T>(dynamic_cast<PropertyArray<T>*>(parrays_[i]));
            return Property<T>();
        }


        // returns a property if it exists, otherwise it creates it first.
        template <class T> Property<T> get_or_add(const std::string& name, const T t=T())
        {
            Property<T> p = get<T>(name);
            if (!p) p = add<T>(name, t);
            return p;
        }


        // get the type of property by its name. returns typeid(void) if it does not exist.
        const std::type_info& get_type(const std::string& name) const
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                if (parrays_[i]->name() == name)
                    return parrays_[i]->type();
            return typeid(void);
        }


        // delete a property. Returns true on success.
        template <class T> bool remove(Property<T>& h)
        {
            std::vector<BasePropertyArray*>::iterator it=parrays_.begin(), end=parrays_.end();
            for (; it!=end; ++it)
            {
                if (*it == h.parray_)
                {
                    delete *it;
                    parrays_.erase(it);
                    h.reset();
                    return true;
                }
            }
            return false;
        }

        // delete a property by name. Returns true on success.
        bool remove(const std::string& name)
        {
            std::vector<BasePropertyArray*>::iterator it=parrays_.begin(), end=parrays_.end();
            for (; it!=end; ++it)
            {
                if ((*it)->name() == name)
                {
                    delete *it;
                    parrays_.erase(it);
                    return true;
                }
            }
            return false;
        }

        // rename a property. Returns true on success.
        bool rename(const std::string& old_name, const std::string& new_name)
        {
            assert(!old_name.empty());
            assert(!new_name.empty());
            std::vector<BasePropertyArray*>::iterator it=parrays_.begin(), end=parrays_.end();
            for (; it!=end; ++it)
            {
                if ((*it)->name() == old_name)
                {
                    (*it)->set_name(new_name);
                    return true;
                }
            }
            return false;
        }


        // delete all properties
        void clear()
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                delete parrays_[i];
            parrays_.clear();
            size_ = 0;
        }


        // reserve memory for n entries in all arrays
        void reserve(size_t n) const
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->reserve(n);
        }

        // resize all arrays to size n
        void resize(size_t n)
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->resize(n);
            size_ = n;
        }

        // resize the vector of properties to n, deleting all other properties
        void resize_property_array(size_t n)
        {
            if (parrays_.size()<=n)
                return;
            for (std::size_t i=n; i<parrays_.size(); ++i)
                delete parrays_[i];
            parrays_.resize(n);
        }

        // free unused space in all arrays
        void shrink_to_fit() const
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->shrink_to_fit();
        }

        // add a new element to each vector
        void push_back()
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->push_back();
            ++size_;
        }

        // reset element to its default property values
        void reset(size_t idx)
        {
            for (std::size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->reset(idx);
        }

        // swap elements i0 and i1 in all arrays
        void swap(size_t i0, size_t i1) const
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->swap(i0, i1);
        }

        // swap content with other Property_container
        void swap (PropertyContainer& other)
        {
            this->parrays_.swap (other.parrays_);
            std::swap(this->size_, other.size_);
        }

        // copy 'from' -> 'to' in all arrays
        void copy(size_t from, size_t to) const
        {
            for (size_t i=0; i<parrays_.size(); ++i)
                parrays_[i]->copy(from, to);
        }

        const std::vector<BasePropertyArray*>& arrays() const { return parrays_; }
        std::vector<BasePropertyArray*>& arrays() { return parrays_; }

    private:
        std::vector<BasePropertyArray*>  parrays_;
        size_t  size_;
    };

} // namespace easy3d

#endif // EASY3D_CORE_PROPERTIES_H
