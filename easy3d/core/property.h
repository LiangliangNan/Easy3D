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
#include <algorithm>
#include <typeinfo>
#include <cassert>

#include <easy3d/util/logging.h>


namespace easy3d {

    /**
     * \brief Base class for a property array.
     * \class BasePropertyArray easy3d/core/property.h
     */
    class BasePropertyArray
    {
    public:
        /**
         * \brief Default constructor.
         * \param name The name of the property array.
         */
        explicit BasePropertyArray(const std::string& name) : name_(name) {}

        /// Destructor.
        virtual ~BasePropertyArray() = default;

        /**
         * \brief Reserves memory for n elements.
         * \param n The number of elements to reserve memory for.
         */
        virtual void reserve(size_t n) = 0;

        /**
         * \brief Resizes storage to hold n elements.
         * \param n The number of elements to resize to.
         */
        virtual void resize(size_t n) = 0;

        /// Free unused memory.
        virtual void shrink_to_fit() = 0;

        /// Extend the number of elements by one.
        virtual void push_back() = 0;

        /**
         * \brief Resets an element to its default value.
         * \param idx The index of the element to reset.
         */
        virtual void reset(size_t idx) = 0;

        /**
         * \brief Copies the entire properties from another property array.
         * \param other The other property array to copy from.
         * \return True if the transfer was successful, false otherwise.
         */
        virtual bool transfer(const BasePropertyArray& other) = 0;
        /**
         * \brief Copies a property from one index to another.
         * \param other The other property array to copy from.
         * \param from The index to copy from.
         * \param to The index to copy to.
         * \return True if the transfer was successful, false otherwise.
         */
        virtual bool transfer(const BasePropertyArray& other, std::size_t from, std::size_t to) = 0;

        /**
         * \brief Swaps the storage place of two elements.
         * \param i0 The index of the first element.
         * \param i1 The index of the second element.
         */
        virtual void swap(size_t i0, size_t i1) = 0;

        /**
         * \brief Copies an element from one index to another.
         * \param from The index to copy from.
         * \param to The index to copy to.
         */
        virtual void copy(size_t from, size_t to) = 0;

        /**
         * \brief Returns a deep copy of the property array.
         * \return A pointer to the deep copy of the property array.
         */
        virtual BasePropertyArray* clone() const = 0;

        /**
         * \brief Returns an empty copy of the property array.
         * \return A pointer to the empty copy of the property array.
         */
        virtual BasePropertyArray* empty_clone() const = 0;

        /**
         * \brief Returns the type information of the property.
         * \return The type information of the property.
         */
        virtual const std::type_info& type() const = 0;

        /**
         * \brief Returns the name of the property.
         * \return The name of the property.
         */
        const std::string& name() const { return name_; }

        /**
         * \brief Sets the name of the property.
         * \param n The new name of the property.
         */
        void set_name(const std::string& n) { name_ = n; }

        /**
         * \brief Tests if two properties are the same.
         * \param other The other property array to compare with.
         * \return True if their names and types are both identical, false otherwise.
         */
        bool is_same(const BasePropertyArray& other) const
        {
            return (name() == other.name() && type() == other.type());
        }

    protected:

        std::string name_;
    };



    //== CLASS DEFINITION =========================================================

    /**
     * \brief Implementation of a generic property array.
     * \class PropertyArray easy3d/core/property.h
     * \tparam T The type of the property.
     */
    template <class T>
    class PropertyArray : public BasePropertyArray
    {
    public:
        typedef T                                      value_type;      ///< The value type of the property.
        typedef std::vector<value_type>                vector_type;     ///< The vector type of the property.
        typedef typename vector_type::reference        reference;       ///< The reference type of the property.
        typedef typename vector_type::const_reference  const_reference; ///< The const reference type of the property.

        /**
         * \brief Constructor.
         * \param name The name of the property array.
         * \param t The default value of the property.
         */
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
        /**
         * \brief Gets a pointer to the array (does not work for T == bool).
         * \return A pointer to the array.
         */
        const T* data() const
        {
            return &data_[0];
        }

        /**
         * \brief Gets a reference to the underlying vector.
         * \return A reference to the underlying vector.
         */
        std::vector<T>& vector()
        {
            return data_;
        }

        /**
         * \brief Accesses the i-th element. No range check is performed.
         * \param _idx The index of the element.
         * \return A reference to the i-th element.
         */
        reference operator[](size_t _idx)
        {
            assert( size_t(_idx) < data_.size() );
            return data_[_idx];
        }

        /**
         * \brief Const access to the i-th element. No range check is performed.
         * \param _idx The index of the element.
         * \return A const reference to the i-th element.
         */
        const_reference operator[](size_t _idx) const
        {
            assert( size_t(_idx) < data_.size());
            return data_[_idx];
        }

    private:
        vector_type data_;
        value_type  value_;
    };


    /**
     * \brief Specialization for bool properties. It crashes the program and simply returns nullptr as bool* is
     *      not allowed.
     */
    template <>
    inline const bool*
    PropertyArray<bool>::data() const
    {
        assert(false);
        return nullptr;
    }



    //== CLASS DEFINITION =========================================================

    /**
     * \brief Implementation of a generic property.
     * \class Property easy3d/core/property.h
     * \tparam T The type of the property.
     */
    template <class T>
    class Property
    {
    public:
        typedef typename PropertyArray<T>::reference reference;             ///< The reference type of the property.
        typedef typename PropertyArray<T>::const_reference const_reference; ///< The const reference type of the property.

        friend class PropertyContainer;

    public:
        /**
         * \brief Constructor.
         * \param p A pointer to the property array.
         */
        explicit Property(PropertyArray<T> *p = nullptr) : parray_(p) {}

        /// Destructor.
        virtual ~Property() = default;

        /**
         * \brief Resets the property.
         */
        void reset()
        {
            parray_ = nullptr;
        }

        /**
         * \brief Checks if the property is valid.
         * \return True if the property is valid, false otherwise.
         */
        operator bool() const
        {
            return parray_ != nullptr;
        }

        /**
         * \brief Accesses the i-th element.
         * \param i The index of the element.
         * \return A reference to the i-th element.
         */
        virtual reference operator[](size_t i)
        {
            assert(parray_ != nullptr);
            return (*parray_)[i];
        }

        /**
         * \brief Const access to the i-th element.
         * \param i The index of the element.
         * \return A const reference to the i-th element.
         */
        virtual const_reference operator[](size_t i) const
        {
            assert(parray_ != nullptr);
            return (*parray_)[i];
        }

        /**
         * \brief Gets a pointer to the array.
         * \return A pointer to the array.
         */
        const T* data() const
        {
            assert(parray_ != nullptr);
            return parray_->data();
        }

        /**
         * \brief Gets a reference to the underlying vector.
         * \return A reference to the underlying vector.
         */
        std::vector<T>& vector()
        {
            assert(parray_ != nullptr);
            return parray_->vector();
        }

        /**
         * \brief Const access to the underlying vector.
         * \return A const reference to the underlying vector.
         */
        const std::vector<T>& vector() const
        {
            assert(parray_ != nullptr);
            return parray_->vector();
        }

        /**
         * \brief Gets a reference to the property array.
         * \return A reference to the property array.
         */
        PropertyArray<T>& array()
        {
            assert(parray_ != nullptr);
            return *parray_;
        }

        /**
         * \brief Const access to the property array.
         * \return A const reference to the property array.
         */
        const PropertyArray<T>& array() const
        {
            assert(parray_ != nullptr);
            return *parray_;
        }

        /**
         * \brief Returns the name of the property.
         * \return The name of the property.
         */
        const std::string& name() const {
            assert(parray_ != nullptr);
            return parray_->name();
        }

        /**
         * \brief Sets the name of the property.
         * \param n The new name of the property.
         */
        void set_name(const std::string& n) {
            assert(parray_ != nullptr);
            parray_->set_name(n);
        }

    private:
        PropertyArray<T>* parray_;
    };


    //== CLASS DEFINITION =========================================================


    /**
     * \brief Implementation of a generic property container.
     * \class PropertyContainer easy3d/core/property.h
     */
    class PropertyContainer
    {
    public:
        /**
         * \brief Default constructor.
         */
        PropertyContainer() : size_(0) {}

        /**
         * \brief Destructor. Deletes all property arrays.
         */
        virtual ~PropertyContainer() { clear(); }

        /**
         * \brief Copy constructor. Performs deep copy of property arrays.
         * \param _rhs The property container to copy from.
         */
        PropertyContainer(const PropertyContainer& _rhs) { operator=(_rhs); }

        /**
         * \brief Assignment operator. Performs deep copy of property arrays.
         * \param _rhs The property container to assign from.
         * \return A reference to the assigned property container.
         */
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

        /**
         * \brief Transfers properties from another property container.
         * \param _rhs The property container to transfer from.
         */
        void transfer(const PropertyContainer& _rhs) const
        {
            for(auto pa : parrays_) {
                for (auto rpa : _rhs.parrays_) {
                    if(pa->is_same(*rpa)){
                        pa->transfer(*rpa);
                        break;
                    }
                }
            }
        }

        /**
         * \brief Copies properties that don't already exist from another container.
         * \param _rhs The property container to copy from.
         */
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

        /**
         * \brief Transfers one element with all properties.
         * \attention Properties must be the same in the two containers
         * \param _rhs The property container to transfer from.
         * \param from The index to transfer from.
         * \param to The index to transfer to.
         * \return True if the transfer was successful, false otherwise.
         */
        bool transfer(const PropertyContainer& _rhs, std::size_t from, std::size_t to) const
        {
            bool out = true;
            for(std::size_t i=0; i<parrays_.size(); ++i)
                if (!(parrays_[i]->transfer(* _rhs.parrays_[i], from, to)))
                    out = false;
            return out;
        }

        /**
         * \brief Returns the current size of the property arrays.
         * \return The current size of the property arrays.
         */
        size_t size() const { return size_; }

        /**
         * \brief Returns the number of property arrays.
         * \return The number of property arrays.
         */
        size_t n_properties() const { return parrays_.size(); }

        /**
         * \brief Returns a vector of all property names.
         * \return A vector of all property names.
         */
        std::vector<std::string> properties() const
        {
            std::vector<std::string> names(parrays_.size());
            for(std::size_t i=0; i<parrays_.size(); ++i)
                names[i] = parrays_[i]->name();
            return names;
        }

        /**
         * \brief Adds a property with a given name and default value.
         * \tparam T The type of the property.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The added property.
         */
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

        /**
         * \brief Gets a property by its name.
         * \tparam T The type of the property.
         * \param name The name of the property.
         * \return The property if it exists, otherwise an invalid property.
         */
        template <class T> Property<T> get(const std::string& name) const
        {
            for(auto pa : parrays_)
                if (pa->name() == name)
                    return Property<T>(dynamic_cast<PropertyArray<T>*>(pa));
            return Property<T>();
        }

        /**
         * \brief Gets or adds a property by its name.
         * \tparam T The type of the property.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The property if it exists, otherwise the added property.
         */
        template <class T> Property<T> get_or_add(const std::string& name, const T t=T())
        {
            Property<T> p = get<T>(name);
            if (!p) p = add<T>(name, t);
            return p;
        }

        /**
         * \brief Gets the type information of a property by its name.
         * \param name The name of the property.
         * \return The type information of the property. Returns a typeid(void) if the property does not exist.
         */
        const std::type_info& get_type(const std::string& name) const
        {
            for(auto pa : parrays_)
                if (pa->name() == name)
                    return pa->type();
            return typeid(void);
        }

        /**
         * \brief Removes a property.
         * \tparam T The type of the property.
         * \param h The property to remove.
         * \return True if the property was removed, false otherwise.
         */
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

        /**
         * \brief Removes a property by its name.
         * \param name The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
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

        /**
         * \brief Renames a property.
         * \param old_name The current name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
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

        /**
         * \brief Deletes all properties.
         */
        void clear()
        {
            for(auto pa : parrays_)
                delete pa;
            parrays_.clear();
            size_ = 0;
        }

        /**
         * \brief Reserves memory for n entries in all arrays.
         * \param n The number of entries to reserve memory for.
         */
        void reserve(size_t n) const
        {
            for(auto pa : parrays_)
                pa->reserve(n);
        }

        /**
         * \brief Resizes all arrays to size n.
         * \param n The new size of the arrays.
         */
        void resize(size_t n)
        {
            for(auto pa : parrays_)
                pa->resize(n);
            size_ = n;
        }

        /**
         * \brief Resizes the vector of properties to n, deleting all other properties.
         * \param n The new size of the property vector.
         */
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

        /**
         * \brief Frees unused space in all arrays.
         */
        void shrink_to_fit() const
        {
            for(auto pa : parrays_)
                pa->shrink_to_fit();
        }

        /**
         * \brief Adds a new element to each vector.
         */
        void push_back()
        {
            for(auto pa : parrays_)
                pa->push_back();
            ++size_;
        }

        /**
         * \brief Resets an element to its default property values.
         * \param idx The index of the element to reset.
         */
        void reset(size_t idx) const {
            for(auto pa : parrays_)
                pa->reset(idx);
        }

        /**
         * \brief Swaps elements i0 and i1 in all arrays.
         * \param i0 The index of the first element.
         * \param i1 The index of the second element.
         */
        void swap(size_t i0, size_t i1) const
        {
            for(auto pa : parrays_)
                pa->swap(i0, i1);
        }

        /**
         * \brief Swaps content with another property container.
         * \param other The other property container to swap with.
         */
        void swap(PropertyContainer& other) noexcept {
            this->parrays_.swap(other.parrays_);
            std::swap(this->size_, other.size_);
        }

        /**
         * \brief Copies an element from one index to another in all arrays.
         * \param from The index to copy from.
         * \param to The index to copy to.
         */
        void copy(size_t from, size_t to) const
        {
            for(auto pa : parrays_)
                pa->copy(from, to);
        }

        /**
         * \brief Returns the vector of property arrays (read-only).
         * \return The vector of property arrays.
         */
        const std::vector<BasePropertyArray*>& arrays() const { return parrays_; }
        /**
         * \brief Returns the vector of property arrays.
         * \return The vector of property arrays.
         */
        std::vector<BasePropertyArray*>& arrays() { return parrays_; }

    private:
        std::vector<BasePropertyArray*>  parrays_;
        size_t  size_;
    };

} // namespace easy3d

#endif // EASY3D_CORE_PROPERTIES_H
