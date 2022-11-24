/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
 * Copyright (C) 2011-2013 by Graphics & Geometry Group, Bielefeld University
 *
 * The code in this file is partly from Surface_mesh (v1.1) with
 * modifications and enhancement:
 *      https://opensource.cit-ec.de/projects/surface_mesh
 * The original code was distributed under the GNU GPL License.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation, version 2.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ********************************************************************/

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
        explicit BasePropertyArray(const std::string& name) : name_(name) {}

        /// Destructor.
        virtual ~BasePropertyArray() = default;

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

        /// Copy the entire properties from \p other.
        virtual bool transfer(const BasePropertyArray& other) = 0;
        /// Copy the property[from] of \p other to this->property[to].
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

        /// Test if two properties are the same.
        /// \return true only if their names and types are both identical.
        bool is_same(const BasePropertyArray& other) const
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

        explicit PropertyArray(const std::string& name, T t=T()) : BasePropertyArray(name), value_(t) {}


    public: // virtual interface of BasePropertyArray

        void reserve(size_t n) override
        {
            data_.reserve(n);
        }

        void resize(size_t n) override
        {
            data_.resize(n, value_);
        }

        void push_back() override
        {
            data_.push_back(value_);
        }

        void reset(size_t idx) override
        {
            data_[idx] = value_;
        }

        bool transfer(const BasePropertyArray& other) override
        {
            const auto pa = dynamic_cast<const PropertyArray*>(&other);
            if(pa != nullptr){
                std::copy((*pa).data_.begin(), (*pa).data_.end(), data_.end()-(*pa).data_.size());
                return true;
            }
            return false;
        }

        bool transfer(const BasePropertyArray& other, std::size_t from, std::size_t to) override
        {
            const auto pa = dynamic_cast<const PropertyArray*>(&other);
            if (pa != nullptr)
            {
                data_[to] = (*pa)[from];
                return true;
            }

            return false;
        }

        void shrink_to_fit() override
        {
            vector_type(data_).swap(data_);
        }

        void swap(size_t i0, size_t i1) override
        {
            T d(data_[i0]);
            data_[i0]=data_[i1];
            data_[i1]=d;
        }

        void copy(size_t from, size_t to) override
        {
            data_[to]=data_[from];
        }

        BasePropertyArray* clone() const override
        {
            auto p = new PropertyArray<T>(name_, value_);
            p->data_ = data_;
            return p;
        }

        BasePropertyArray* empty_clone() const override
        {
            auto p = new PropertyArray<T>(this->name_, this->value_);
            return p;
        }

        const std::type_info& type() const override { return typeid(T); }


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

        explicit Property(PropertyArray<T> *p = nullptr) : parray_(p) {}

        void reset()
        {
            parray_ = nullptr;
        }

        operator bool() const
        {
            return parray_ != nullptr;
        }

        virtual reference operator[](size_t i)
        {
            assert(parray_ != nullptr);
            return (*parray_)[i];
        }

        virtual const_reference operator[](size_t i) const
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
            for(auto pa : parrays_) {
                for (auto rpa : _rhs.parrays_) {
                    if(pa->is_same (*rpa)){
                        pa->transfer(*rpa);
                        break;
                    }
                }
            }
        }

        // Copy properties that don't already exist from another container
        void copy_properties(const PropertyContainer& _rhs)
        {
            for (auto rpa : _rhs.parrays_)
            {
                bool property_already_exists = false;
                for(auto pa : parrays_)
                    if (rpa->is_same(*pa))
                    {
                        property_already_exists = true;
                        break;
                    }

                if (property_already_exists)
                    continue;

                parrays_.push_back(rpa->empty_clone());
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
            for(auto pa : parrays_)
                names.push_back(pa->name());
            return names;
        }


        // add a property with name \c name and default value \c t
        template <class T> Property<T> add(const std::string& name, const T t=T())
        {
            // if a property with this name already exists, return an invalid property
            for(auto pa : parrays_)
            {
                if (pa->name() == name)
                {
                    LOG(ERROR) << "A property with name \""
                              << name << "\" already exists. Returning invalid property.";
                    return Property<T>();
                }
            }

            // otherwise add the property
            auto p = new PropertyArray<T>(name, t);
            p->resize(size_);
            parrays_.push_back(p);
            return Property<T>(p);
        }


        // get a property by its name. returns invalid property if it does not exist.
        template <class T> Property<T> get(const std::string& name) const
        {
            for(auto pa : parrays_)
                if (pa->name() == name)
                    return Property<T>(dynamic_cast<PropertyArray<T>*>(pa));
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
            for(auto pa : parrays_)
                if (pa->name() == name)
                    return pa->type();
            return typeid(void);
        }


        // delete a property. Returns true on success.
        template <class T> bool remove(Property<T>& h)
        {
            auto it=parrays_.begin(), end=parrays_.end();
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
            auto it=parrays_.begin(), end=parrays_.end();
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
            auto it=parrays_.begin(), end=parrays_.end();
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
            for(auto pa : parrays_)
                delete pa;
            parrays_.clear();
            size_ = 0;
        }


        // reserve memory for n entries in all arrays
        void reserve(size_t n) const
        {
            for(auto pa : parrays_)
                pa->reserve(n);
        }

        // resize all arrays to size n
        void resize(size_t n)
        {
            for(auto pa : parrays_)
                pa->resize(n);
            size_ = n;
        }

        // resize the vector of properties to n, deleting all other properties
        void resize_property_array(size_t n)
        {
            if (parrays_.size()<=n) {
                // Liangliang: we should add "parrays_.resize(n);" here?
                return;
            }
            for (std::size_t i=n; i<parrays_.size(); ++i)
                delete parrays_[i];
            parrays_.resize(n);
        }

        // free unused space in all arrays
        void shrink_to_fit() const
        {
            for(auto pa : parrays_)
                pa->shrink_to_fit();
        }

        // add a new element to each vector
        void push_back()
        {
            for(auto pa : parrays_)
                pa->push_back();
            ++size_;
        }

        // reset element to its default property values
        void reset(size_t idx)
        {
            for(auto pa : parrays_)
                pa->reset(idx);
        }

        // swap elements i0 and i1 in all arrays
        void swap(size_t i0, size_t i1) const
        {
            for(auto pa : parrays_)
                pa->swap(i0, i1);
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
            for(auto pa : parrays_)
                pa->copy(from, to);
        }

        const std::vector<BasePropertyArray*>& arrays() const { return parrays_; }
        std::vector<BasePropertyArray*>& arrays() { return parrays_; }

    private:
        std::vector<BasePropertyArray*>  parrays_;
        size_t  size_;
    };

} // namespace easy3d

#endif // EASY3D_CORE_PROPERTIES_H
