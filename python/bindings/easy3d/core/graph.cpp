#include <easy3d/core/graph.h>
#include <easy3d/core/property.h>
#include <easy3d/core/vec.h>

#include <memory>
#include <typeinfo>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


// easy3d::Graph file:easy3d/core/graph.h line:50
struct PyCallBack_easy3d_Graph : public easy3d::Graph {
    using easy3d::Graph::Graph;

    void property_stats(std::ostream & a0) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Graph *>(this), "property_stats");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return Graph::property_stats(a0);
    }
    using _binder_ret_0 = std::vector<class easy3d::Vec<3, float> > &;
    _binder_ret_0 points() override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Graph *>(this), "points");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<_binder_ret_0>::value) {
                static pybind11::detail::override_caster_t<_binder_ret_0> caster;
                return pybind11::detail::cast_ref<_binder_ret_0>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<_binder_ret_0>(std::move(o));
        }
        return Graph::points();
    }
};

// easy3d::Graph::VertexProperty file:easy3d/core/graph.h line:142
struct PyCallBack_easy3d_Graph_VertexProperty_easy3d_Graph_VertexConnectivity_t : public easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity> {
    using easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::VertexProperty;

    struct easy3d::Graph::VertexConnectivity & operator[](unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::Graph::VertexConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::Graph::VertexConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::Graph::VertexConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::Graph::VertexConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Graph::VertexProperty file:easy3d/core/graph.h line:142
struct PyCallBack_easy3d_Graph_VertexProperty_easy3d_Vec_3_float_t : public easy3d::Graph::VertexProperty<easy3d::Vec<3, float>> {
    using easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::VertexProperty;

    using _binder_ret_0 = easy3d::Vec<3, float> &;
    _binder_ret_0 operator[](unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Graph::VertexProperty<easy3d::Vec<3, float>> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<_binder_ret_0>::value) {
                static pybind11::detail::override_caster_t<_binder_ret_0> caster;
                return pybind11::detail::cast_ref<_binder_ret_0>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<_binder_ret_0>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Graph::EdgeProperty file:easy3d/core/graph.h line:166
struct PyCallBack_easy3d_Graph_EdgeProperty_easy3d_Graph_EdgeConnectivity_t : public easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity> {
    using easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::EdgeProperty;

    struct easy3d::Graph::EdgeConnectivity & operator[](unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::Graph::EdgeConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::Graph::EdgeConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::Graph::EdgeConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::Graph::EdgeConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_Graph_VertexConnectivity_t : public easy3d::Property<easy3d::Graph::VertexConnectivity> {
    using easy3d::Property<easy3d::Graph::VertexConnectivity>::Property;

    struct easy3d::Graph::VertexConnectivity & operator[](unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::Graph::VertexConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::Graph::VertexConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::Graph::VertexConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::Graph::VertexConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::Graph::VertexConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_Graph_EdgeConnectivity_t : public easy3d::Property<easy3d::Graph::EdgeConnectivity> {
    using easy3d::Property<easy3d::Graph::EdgeConnectivity>::Property;

    struct easy3d::Graph::EdgeConnectivity & operator[](unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::Graph::EdgeConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::Graph::EdgeConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::Graph::EdgeConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::Graph::EdgeConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::Graph::EdgeConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_easy3d_Graph_VertexConnectivity_t : public easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> {
    using easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::PropertyArray;

    void reserve(unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "reserve");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::reserve(a0);
    }
    void resize(unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "resize");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::resize(a0);
    }
    void push_back() override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "push_back");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::push_back();
    }
    void reset(unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "reset");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::reset(a0);
    }
    bool transfer(const class easy3d::BasePropertyArray & a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "transfer");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
                static pybind11::detail::override_caster_t<bool> caster;
                return pybind11::detail::cast_ref<bool>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<bool>(std::move(o));
        }
        return PropertyArray::transfer(a0);
    }
    bool transfer(const class easy3d::BasePropertyArray & a0, std::size_t a1, std::size_t a2) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "transfer");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
                static pybind11::detail::override_caster_t<bool> caster;
                return pybind11::detail::cast_ref<bool>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<bool>(std::move(o));
        }
        return PropertyArray::transfer(a0, a1, a2);
    }
    void shrink_to_fit() override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "shrink_to_fit");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::shrink_to_fit();
    }
    void swap(unsigned long a0, unsigned long a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "swap");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::swap(a0, a1);
    }
    void copy(unsigned long a0, unsigned long a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "copy");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::copy(a0, a1);
    }
    class easy3d::BasePropertyArray * clone() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "clone");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::BasePropertyArray *>::value) {
                static pybind11::detail::override_caster_t<class easy3d::BasePropertyArray *> caster;
                return pybind11::detail::cast_ref<class easy3d::BasePropertyArray *>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<class easy3d::BasePropertyArray *>(std::move(o));
        }
        return PropertyArray::clone();
    }
    class easy3d::BasePropertyArray * empty_clone() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "empty_clone");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::BasePropertyArray *>::value) {
                static pybind11::detail::override_caster_t<class easy3d::BasePropertyArray *> caster;
                return pybind11::detail::cast_ref<class easy3d::BasePropertyArray *>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<class easy3d::BasePropertyArray *>(std::move(o));
        }
        return PropertyArray::empty_clone();
    }
    const class std::type_info & type() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> *>(this), "type");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<const class std::type_info &>::value) {
                static pybind11::detail::override_caster_t<const class std::type_info &> caster;
                return pybind11::detail::cast_ref<const class std::type_info &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<const class std::type_info &>(std::move(o));
        }
        return PropertyArray::type();
    }
};

// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_easy3d_Graph_EdgeConnectivity_t : public easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> {
    using easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::PropertyArray;

    void reserve(unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "reserve");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::reserve(a0);
    }
    void resize(unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "resize");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::resize(a0);
    }
    void push_back() override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "push_back");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::push_back();
    }
    void reset(unsigned long a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "reset");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::reset(a0);
    }
    bool transfer(const class easy3d::BasePropertyArray & a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "transfer");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
                static pybind11::detail::override_caster_t<bool> caster;
                return pybind11::detail::cast_ref<bool>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<bool>(std::move(o));
        }
        return PropertyArray::transfer(a0);
    }
    bool transfer(const class easy3d::BasePropertyArray & a0, std::size_t a1, std::size_t a2) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "transfer");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
                static pybind11::detail::override_caster_t<bool> caster;
                return pybind11::detail::cast_ref<bool>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<bool>(std::move(o));
        }
        return PropertyArray::transfer(a0, a1, a2);
    }
    void shrink_to_fit() override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "shrink_to_fit");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::shrink_to_fit();
    }
    void swap(unsigned long a0, unsigned long a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "swap");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::swap(a0, a1);
    }
    void copy(unsigned long a0, unsigned long a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "copy");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return PropertyArray::copy(a0, a1);
    }
    class easy3d::BasePropertyArray * clone() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "clone");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::BasePropertyArray *>::value) {
                static pybind11::detail::override_caster_t<class easy3d::BasePropertyArray *> caster;
                return pybind11::detail::cast_ref<class easy3d::BasePropertyArray *>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<class easy3d::BasePropertyArray *>(std::move(o));
        }
        return PropertyArray::clone();
    }
    class easy3d::BasePropertyArray * empty_clone() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "empty_clone");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::BasePropertyArray *>::value) {
                static pybind11::detail::override_caster_t<class easy3d::BasePropertyArray *> caster;
                return pybind11::detail::cast_ref<class easy3d::BasePropertyArray *>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<class easy3d::BasePropertyArray *>(std::move(o));
        }
        return PropertyArray::empty_clone();
    }
    const class std::type_info & type() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> *>(this), "type");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<const class std::type_info &>::value) {
                static pybind11::detail::override_caster_t<const class std::type_info &> caster;
                return pybind11::detail::cast_ref<const class std::type_info &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<const class std::type_info &>(std::move(o));
        }
        return PropertyArray::type();
    }
};


void bind_easy3d_core_graph(pybind11::module_& m)
{
	{ // easy3d::Graph file:easy3d/core/graph.h line:50
		pybind11::class_<easy3d::Graph, std::shared_ptr<easy3d::Graph>, PyCallBack_easy3d_Graph, easy3d::Model> cl(m, "Graph", "A Graph data structure with easy property management.\n \n\n\n This implementation is inspired by Surface_mesh\n https://opensource.cit-ec.de/projects/surface_mesh");
		cl.def( pybind11::init( [](){ return new easy3d::Graph(); }, [](){ return new PyCallBack_easy3d_Graph(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_Graph const &o){ return new PyCallBack_easy3d_Graph(o); } ) );
		cl.def( pybind11::init( [](easy3d::Graph const &o){ return new easy3d::Graph(o); } ) );
		cl.def("assign", (class easy3d::Graph & (easy3d::Graph::*)(const class easy3d::Graph &)) &easy3d::Graph::operator=, "assign  to  performs a deep copy of all properties.\n\nC++: easy3d::Graph::operator=(const class easy3d::Graph &) --> class easy3d::Graph &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("assign", (class easy3d::Graph & (easy3d::Graph::*)(const class easy3d::Graph &)) &easy3d::Graph::assign, "assign  to  does not copy custom properties.\n\nC++: easy3d::Graph::assign(const class easy3d::Graph &) --> class easy3d::Graph &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("add_vertex", (struct easy3d::Graph::Vertex (easy3d::Graph::*)(const class easy3d::Vec<3, float> &)) &easy3d::Graph::add_vertex, "add a new vertex with position \n\nC++: easy3d::Graph::add_vertex(const class easy3d::Vec<3, float> &) --> struct easy3d::Graph::Vertex", pybind11::arg("p"));
		cl.def("add_edge", (struct easy3d::Graph::Edge (easy3d::Graph::*)(const struct easy3d::Graph::Vertex &, const struct easy3d::Graph::Vertex &)) &easy3d::Graph::add_edge, "add a new edge connecting vertices  and \n\nC++: easy3d::Graph::add_edge(const struct easy3d::Graph::Vertex &, const struct easy3d::Graph::Vertex &) --> struct easy3d::Graph::Edge", pybind11::arg("v1"), pybind11::arg("v2"));
		cl.def("vertices_size", (unsigned int (easy3d::Graph::*)() const) &easy3d::Graph::vertices_size, "returns number of (deleted and valid) vertices in the graph\n\nC++: easy3d::Graph::vertices_size() const --> unsigned int");
		cl.def("edges_size", (unsigned int (easy3d::Graph::*)() const) &easy3d::Graph::edges_size, "returns number of (deleted and valid)edges in the graph\n\nC++: easy3d::Graph::edges_size() const --> unsigned int");
		cl.def("n_vertices", (unsigned int (easy3d::Graph::*)() const) &easy3d::Graph::n_vertices, "returns number of vertices in the graph\n\nC++: easy3d::Graph::n_vertices() const --> unsigned int");
		cl.def("n_edges", (unsigned int (easy3d::Graph::*)() const) &easy3d::Graph::n_edges, "returns number of edges in the graph\n\nC++: easy3d::Graph::n_edges() const --> unsigned int");
		cl.def("clear", (void (easy3d::Graph::*)()) &easy3d::Graph::clear, "Removes all vertices, edges, and properties (and resets garbage state).\n \n\n After calling this method, the graph is the same as newly constructed.\n\nC++: easy3d::Graph::clear() --> void");
		cl.def("reserve", (void (easy3d::Graph::*)(unsigned int, unsigned int)) &easy3d::Graph::reserve, "reserve memory (mainly used in file readers)\n\nC++: easy3d::Graph::reserve(unsigned int, unsigned int) --> void", pybind11::arg("nvertices"), pybind11::arg("nedges"));
		cl.def("resize", (void (easy3d::Graph::*)(unsigned int, unsigned int)) &easy3d::Graph::resize, "Resize space for vertices, edges, and their currently associated properties.\n Note: ne is the number of edges.\n\nC++: easy3d::Graph::resize(unsigned int, unsigned int) --> void", pybind11::arg("nv"), pybind11::arg("ne"));
		cl.def("has_garbage", (bool (easy3d::Graph::*)() const) &easy3d::Graph::has_garbage, "are there deleted vertices or edges?\n\nC++: easy3d::Graph::has_garbage() const --> bool");
		cl.def("collect_garbage", (void (easy3d::Graph::*)()) &easy3d::Graph::collect_garbage, "remove deleted vertices/edges\n\nC++: easy3d::Graph::collect_garbage() --> void");
		cl.def("is_deleted", (bool (easy3d::Graph::*)(struct easy3d::Graph::Vertex) const) &easy3d::Graph::is_deleted, "returns whether vertex  is deleted\n \n\n collect_garbage()\n\nC++: easy3d::Graph::is_deleted(struct easy3d::Graph::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_deleted", (bool (easy3d::Graph::*)(struct easy3d::Graph::Edge) const) &easy3d::Graph::is_deleted, "returns whether edge  is deleted\n \n\n collect_garbage()\n\nC++: easy3d::Graph::is_deleted(struct easy3d::Graph::Edge) const --> bool", pybind11::arg("e"));
		cl.def("is_valid", (bool (easy3d::Graph::*)(struct easy3d::Graph::Vertex) const) &easy3d::Graph::is_valid, "return whether vertex  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::Graph::is_valid(struct easy3d::Graph::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_valid", (bool (easy3d::Graph::*)(struct easy3d::Graph::Edge) const) &easy3d::Graph::is_valid, "return whether edge  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::Graph::is_valid(struct easy3d::Graph::Edge) const --> bool", pybind11::arg("e"));
		cl.def("is_isolated", (bool (easy3d::Graph::*)(struct easy3d::Graph::Vertex) const) &easy3d::Graph::is_isolated, "returns whether  is isolated, i.e., not incident to any edge\n\nC++: easy3d::Graph::is_isolated(struct easy3d::Graph::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("vertex", (struct easy3d::Graph::Vertex (easy3d::Graph::*)(struct easy3d::Graph::Edge, unsigned int) const) &easy3d::Graph::vertex, "returns the  vertex of edge   has to be 0 or 1.\n\nC++: easy3d::Graph::vertex(struct easy3d::Graph::Edge, unsigned int) const --> struct easy3d::Graph::Vertex", pybind11::arg("e"), pybind11::arg("i"));
		cl.def("source", (struct easy3d::Graph::Vertex (easy3d::Graph::*)(struct easy3d::Graph::Edge) const) &easy3d::Graph::source, "returns the starting vertex of an edge, which is equal to vertex(e, 0).\n\nC++: easy3d::Graph::source(struct easy3d::Graph::Edge) const --> struct easy3d::Graph::Vertex", pybind11::arg("e"));
		cl.def("target", (struct easy3d::Graph::Vertex (easy3d::Graph::*)(struct easy3d::Graph::Edge) const) &easy3d::Graph::target, "returns the ending vertex of an edge, which is equal to vertex(e, 1).\n\nC++: easy3d::Graph::target(struct easy3d::Graph::Edge) const --> struct easy3d::Graph::Vertex", pybind11::arg("e"));
		cl.def("remove_vertex_property", (bool (easy3d::Graph::*)(const std::string &)) &easy3d::Graph::remove_vertex_property, "remove the vertex property named \n\nC++: easy3d::Graph::remove_vertex_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_edge_property", (bool (easy3d::Graph::*)(const std::string &)) &easy3d::Graph::remove_edge_property, "remove the edge property named \n\nC++: easy3d::Graph::remove_edge_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_model_property", (bool (easy3d::Graph::*)(const std::string &)) &easy3d::Graph::remove_model_property, "remove the model property named \n\nC++: easy3d::Graph::remove_model_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("rename_vertex_property", (bool (easy3d::Graph::*)(const std::string &, const std::string &)) &easy3d::Graph::rename_vertex_property, "rename a vertex property given its name\n\nC++: easy3d::Graph::rename_vertex_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_edge_property", (bool (easy3d::Graph::*)(const std::string &, const std::string &)) &easy3d::Graph::rename_edge_property, "rename an edge property given its name\n\nC++: easy3d::Graph::rename_edge_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_model_property", (bool (easy3d::Graph::*)(const std::string &, const std::string &)) &easy3d::Graph::rename_model_property, "rename a model property given its name\n\nC++: easy3d::Graph::rename_model_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("get_vertex_property_type", (const class std::type_info & (easy3d::Graph::*)(const std::string &) const) &easy3d::Graph::get_vertex_property_type, "get the type_info  of vertex property  returns an typeid(void)\n		 if the property does not exist or if the type does not match. \n\nC++: easy3d::Graph::get_vertex_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_edge_property_type", (const class std::type_info & (easy3d::Graph::*)(const std::string &) const) &easy3d::Graph::get_edge_property_type, "get the type_info  of edge property  returns an typeid(void)\n		 if the property does not exist or if the type does not match. \n\nC++: easy3d::Graph::get_edge_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_model_property_type", (const class std::type_info & (easy3d::Graph::*)(const std::string &) const) &easy3d::Graph::get_model_property_type, "get the type_info  of model property  returns an typeid(void)\n		 if the property does not exist or if the type does not match. \n\nC++: easy3d::Graph::get_model_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("vertex_properties", (class std::vector<std::string> (easy3d::Graph::*)() const) &easy3d::Graph::vertex_properties, "returns the names of all vertex properties\n\nC++: easy3d::Graph::vertex_properties() const --> class std::vector<std::string>");
		cl.def("edge_properties", (class std::vector<std::string> (easy3d::Graph::*)() const) &easy3d::Graph::edge_properties, "returns the names of all edge properties\n\nC++: easy3d::Graph::edge_properties() const --> class std::vector<std::string>");
		cl.def("model_properties", (class std::vector<std::string> (easy3d::Graph::*)() const) &easy3d::Graph::model_properties, "returns the names of all model properties\n\nC++: easy3d::Graph::model_properties() const --> class std::vector<std::string>");
		cl.def("property_stats", (void (easy3d::Graph::*)(std::ostream &) const) &easy3d::Graph::property_stats, "prints the names of all properties to an output stream (e.g., std::cout)\n\nC++: easy3d::Graph::property_stats(std::ostream &) const --> void", pybind11::arg("output"));
		cl.def("vertices_begin", (class easy3d::Graph::VertexIterator (easy3d::Graph::*)() const) &easy3d::Graph::vertices_begin, "returns start iterator for vertices\n\nC++: easy3d::Graph::vertices_begin() const --> class easy3d::Graph::VertexIterator");
		cl.def("vertices_end", (class easy3d::Graph::VertexIterator (easy3d::Graph::*)() const) &easy3d::Graph::vertices_end, "returns end iterator for vertices\n\nC++: easy3d::Graph::vertices_end() const --> class easy3d::Graph::VertexIterator");
		cl.def("vertices", (class easy3d::Graph::VertexContainer (easy3d::Graph::*)() const) &easy3d::Graph::vertices, "returns vertex container for C++11 range-based for-loops\n\nC++: easy3d::Graph::vertices() const --> class easy3d::Graph::VertexContainer");
		cl.def("edges_begin", (class easy3d::Graph::EdgeIterator (easy3d::Graph::*)() const) &easy3d::Graph::edges_begin, "returns start iterator for edges\n\nC++: easy3d::Graph::edges_begin() const --> class easy3d::Graph::EdgeIterator");
		cl.def("edges_end", (class easy3d::Graph::EdgeIterator (easy3d::Graph::*)() const) &easy3d::Graph::edges_end, "returns end iterator for edges\n\nC++: easy3d::Graph::edges_end() const --> class easy3d::Graph::EdgeIterator");
		cl.def("edges", (class easy3d::Graph::EdgeContainer (easy3d::Graph::*)() const) &easy3d::Graph::edges, "returns edge container for C++11 range-based for-loops\n\nC++: easy3d::Graph::edges() const --> class easy3d::Graph::EdgeContainer");
		cl.def("vertices", (class easy3d::Graph::VertexAroundVertexCirculator (easy3d::Graph::*)(struct easy3d::Graph::Vertex) const) &easy3d::Graph::vertices, "returns circulator for vertices around vertex \n\nC++: easy3d::Graph::vertices(struct easy3d::Graph::Vertex) const --> class easy3d::Graph::VertexAroundVertexCirculator", pybind11::arg("v"));
		cl.def("edges", (class easy3d::Graph::EdgeAroundVertexCirculator (easy3d::Graph::*)(struct easy3d::Graph::Vertex) const) &easy3d::Graph::edges, "returns circulator for edges around vertex \n\nC++: easy3d::Graph::edges(struct easy3d::Graph::Vertex) const --> class easy3d::Graph::EdgeAroundVertexCirculator", pybind11::arg("v"));
		cl.def("valence", (unsigned int (easy3d::Graph::*)(struct easy3d::Graph::Vertex) const) &easy3d::Graph::valence, "returns the valence (number of incident edges or neighboring vertices)\n		 of vertex  \n\nC++: easy3d::Graph::valence(struct easy3d::Graph::Vertex) const --> unsigned int", pybind11::arg("v"));
		cl.def("find_edge", (struct easy3d::Graph::Edge (easy3d::Graph::*)(struct easy3d::Graph::Vertex, struct easy3d::Graph::Vertex) const) &easy3d::Graph::find_edge, "find the edge (a,b)\n\nC++: easy3d::Graph::find_edge(struct easy3d::Graph::Vertex, struct easy3d::Graph::Vertex) const --> struct easy3d::Graph::Edge", pybind11::arg("a"), pybind11::arg("b"));
		cl.def("delete_vertex", (void (easy3d::Graph::*)(struct easy3d::Graph::Vertex)) &easy3d::Graph::delete_vertex, "deletes the vertex  from the graph\n\nC++: easy3d::Graph::delete_vertex(struct easy3d::Graph::Vertex) --> void", pybind11::arg("v"));
		cl.def("delete_edge", (void (easy3d::Graph::*)(struct easy3d::Graph::Edge)) &easy3d::Graph::delete_edge, "deletes the edge  from the graph\n\nC++: easy3d::Graph::delete_edge(struct easy3d::Graph::Edge) --> void", pybind11::arg("e"));
		cl.def("position", (class easy3d::Vec<3, float> & (easy3d::Graph::*)(struct easy3d::Graph::Vertex)) &easy3d::Graph::position, "position of a vertex\n\nC++: easy3d::Graph::position(struct easy3d::Graph::Vertex) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("points", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Graph::*)()) &easy3d::Graph::points, "vector of vertex positions\n\nC++: easy3d::Graph::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
		cl.def("edge_length", (float (easy3d::Graph::*)(struct easy3d::Graph::Edge) const) &easy3d::Graph::edge_length, "compute the length of edge \n\nC++: easy3d::Graph::edge_length(struct easy3d::Graph::Edge) const --> float", pybind11::arg("e"));

		{ // easy3d::Graph::BaseHandle file:easy3d/core/graph.h line:58
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::BaseHandle, std::shared_ptr<easy3d::Graph::BaseHandle>> cl(enclosing_class, "BaseHandle", "Base class for all topology types (internally it is basically an index)\n \n\n Vertex, Edge");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::BaseHandle(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::Graph::BaseHandle const &o){ return new easy3d::Graph::BaseHandle(o); } ) );
			cl.def("idx", (int (easy3d::Graph::BaseHandle::*)() const) &easy3d::Graph::BaseHandle::idx, "Get the underlying index of this handle\n\nC++: easy3d::Graph::BaseHandle::idx() const --> int");
			cl.def("reset", (void (easy3d::Graph::BaseHandle::*)()) &easy3d::Graph::BaseHandle::reset, "reset handle to be invalid (index=-1)\n\nC++: easy3d::Graph::BaseHandle::reset() --> void");
			cl.def("is_valid", (bool (easy3d::Graph::BaseHandle::*)() const) &easy3d::Graph::BaseHandle::is_valid, "return whether the handle is valid, i.e., the index is not equal to -1.\n\nC++: easy3d::Graph::BaseHandle::is_valid() const --> bool");
			cl.def("__eq__", (bool (easy3d::Graph::BaseHandle::*)(const class easy3d::Graph::BaseHandle &) const) &easy3d::Graph::BaseHandle::operator==, "are two handles equal?\n\nC++: easy3d::Graph::BaseHandle::operator==(const class easy3d::Graph::BaseHandle &) const --> bool", pybind11::arg("_rhs"));
			cl.def("__ne__", (bool (easy3d::Graph::BaseHandle::*)(const class easy3d::Graph::BaseHandle &) const) &easy3d::Graph::BaseHandle::operator!=, "are two handles different?\n\nC++: easy3d::Graph::BaseHandle::operator!=(const class easy3d::Graph::BaseHandle &) const --> bool", pybind11::arg("_rhs"));
			cl.def("assign", (class easy3d::Graph::BaseHandle & (easy3d::Graph::BaseHandle::*)(const class easy3d::Graph::BaseHandle &)) &easy3d::Graph::BaseHandle::operator=, "C++: easy3d::Graph::BaseHandle::operator=(const class easy3d::Graph::BaseHandle &) --> class easy3d::Graph::BaseHandle &", pybind11::return_value_policy::automatic, pybind11::arg(""));

			{ // easy3d::Graph::BaseHandle::Hash file:easy3d/core/graph.h line:90
				auto & enclosing_class = cl;
				pybind11::class_<easy3d::Graph::BaseHandle::Hash, std::shared_ptr<easy3d::Graph::BaseHandle::Hash>> cl(enclosing_class, "Hash", "helper structure to be able to use std::unordered_map");
				cl.def( pybind11::init( [](){ return new easy3d::Graph::BaseHandle::Hash(); } ) );
				cl.def("__call__", (std::size_t (easy3d::Graph::BaseHandle::Hash::*)(const class easy3d::Graph::BaseHandle &) const) &easy3d::Graph::BaseHandle::Hash::operator(), "C++: easy3d::Graph::BaseHandle::Hash::operator()(const class easy3d::Graph::BaseHandle &) const --> std::size_t", pybind11::arg("h"));
			}

		}

		{ // easy3d::Graph::Vertex file:easy3d/core/graph.h line:102
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::Vertex, std::shared_ptr<easy3d::Graph::Vertex>, easy3d::Graph::BaseHandle> cl(enclosing_class, "Vertex", "this type represents a vertex (internally it is basically an index)\n  \n\n Edge");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::Vertex(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::Graph::Vertex const &o){ return new easy3d::Graph::Vertex(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::Graph::Vertex::*)(std::ostream &) const) &easy3d::Graph::Vertex::operator<<, "C++: easy3d::Graph::Vertex::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::Graph::Vertex & (easy3d::Graph::Vertex::*)(const struct easy3d::Graph::Vertex &)) &easy3d::Graph::Vertex::operator=, "C++: easy3d::Graph::Vertex::operator=(const struct easy3d::Graph::Vertex &) --> struct easy3d::Graph::Vertex &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::Graph::Edge file:easy3d/core/graph.h line:111
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::Edge, std::shared_ptr<easy3d::Graph::Edge>, easy3d::Graph::BaseHandle> cl(enclosing_class, "Edge", "this type represents an edge (internally it is basically an index)\n \n\n Vertex");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::Edge(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::Graph::Edge const &o){ return new easy3d::Graph::Edge(o); } ) );
		}

		{ // easy3d::Graph::VertexConnectivity file:easy3d/core/graph.h line:122
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexConnectivity, std::shared_ptr<easy3d::Graph::VertexConnectivity>> cl(enclosing_class, "VertexConnectivity", "This type stores the vertex connectivity\n \n\n EdgeConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::VertexConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::Graph::VertexConnectivity const &o){ return new easy3d::Graph::VertexConnectivity(o); } ) );
			cl.def_readwrite("edges_", &easy3d::Graph::VertexConnectivity::edges_);
			cl.def("assign", (struct easy3d::Graph::VertexConnectivity & (easy3d::Graph::VertexConnectivity::*)(const struct easy3d::Graph::VertexConnectivity &)) &easy3d::Graph::VertexConnectivity::operator=, "C++: easy3d::Graph::VertexConnectivity::operator=(const struct easy3d::Graph::VertexConnectivity &) --> struct easy3d::Graph::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::Graph::EdgeConnectivity file:easy3d/core/graph.h line:131
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::EdgeConnectivity, std::shared_ptr<easy3d::Graph::EdgeConnectivity>> cl(enclosing_class, "EdgeConnectivity", "This type stores the edge connectivity\n \n\n VertexConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::EdgeConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::Graph::EdgeConnectivity const &o){ return new easy3d::Graph::EdgeConnectivity(o); } ) );
			cl.def_readwrite("source_", &easy3d::Graph::EdgeConnectivity::source_);
			cl.def_readwrite("target_", &easy3d::Graph::EdgeConnectivity::target_);
			cl.def("assign", (struct easy3d::Graph::EdgeConnectivity & (easy3d::Graph::EdgeConnectivity::*)(const struct easy3d::Graph::EdgeConnectivity &)) &easy3d::Graph::EdgeConnectivity::operator=, "C++: easy3d::Graph::EdgeConnectivity::operator=(const struct easy3d::Graph::EdgeConnectivity &) --> struct easy3d::Graph::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::Graph::VertexConnectivity>, std::shared_ptr<easy3d::Property<easy3d::Graph::VertexConnectivity>>, PyCallBack_easy3d_Property_easy3d_Graph_VertexConnectivity_t> cl(m, "Property_easy3d_Graph_VertexConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::Graph::VertexConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_Graph_VertexConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_Graph_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_Graph_VertexConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::Graph::VertexConnectivity> const &o){ return new easy3d::Property<easy3d::Graph::VertexConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::Property<easy3d::Graph::VertexConnectivity>::reset, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::Graph::VertexConnectivity & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)(unsigned long)) &easy3d::Property<easy3d::Graph::VertexConnectivity>::operator[], "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::operator[](unsigned long) --> struct easy3d::Graph::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::Graph::VertexConnectivity * (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::VertexConnectivity>::data, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::data() const --> const struct easy3d::Graph::VertexConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::Graph::VertexConnectivity> & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::Property<easy3d::Graph::VertexConnectivity>::vector, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::vector() --> class std::vector<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::Property<easy3d::Graph::VertexConnectivity>::array, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::VertexConnectivity>::name, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::Graph::VertexConnectivity>::set_name, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::Graph::VertexConnectivity> & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)(const class easy3d::Property<struct easy3d::Graph::VertexConnectivity> &)) &easy3d::Property<easy3d::Graph::VertexConnectivity>::operator=, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::operator=(const class easy3d::Property<struct easy3d::Graph::VertexConnectivity> &) --> class easy3d::Property<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::Graph::VertexProperty file:easy3d/core/graph.h line:142
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>, std::shared_ptr<easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>>, PyCallBack_easy3d_Graph_VertexProperty_easy3d_Graph_VertexConnectivity_t, easy3d::Property<easy3d::Graph::VertexConnectivity>> cl(enclosing_class, "VertexProperty_easy3d_Graph_VertexConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>(); }, [](){ return new PyCallBack_easy3d_Graph_VertexProperty_easy3d_Graph_VertexConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::Graph::VertexConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_Graph_VertexProperty_easy3d_Graph_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_Graph_VertexProperty_easy3d_Graph_VertexConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity> const &o){ return new easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::Graph::VertexConnectivity & (easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::*)(struct easy3d::Graph::Vertex)) &easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::operator[], "C++: easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::operator[](struct easy3d::Graph::Vertex) --> struct easy3d::Graph::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::Graph::VertexProperty<struct easy3d::Graph::VertexConnectivity> & (easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::*)(const class easy3d::Graph::VertexProperty<struct easy3d::Graph::VertexConnectivity> &)) &easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::operator=, "C++: easy3d::Graph::VertexProperty<easy3d::Graph::VertexConnectivity>::operator=(const class easy3d::Graph::VertexProperty<struct easy3d::Graph::VertexConnectivity> &) --> class easy3d::Graph::VertexProperty<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::Property<easy3d::Graph::VertexConnectivity>::reset, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::Graph::VertexConnectivity & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)(unsigned long)) &easy3d::Property<easy3d::Graph::VertexConnectivity>::operator[], "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::operator[](unsigned long) --> struct easy3d::Graph::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::Graph::VertexConnectivity * (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::VertexConnectivity>::data, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::data() const --> const struct easy3d::Graph::VertexConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::Graph::VertexConnectivity> & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::Property<easy3d::Graph::VertexConnectivity>::vector, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::vector() --> class std::vector<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::Property<easy3d::Graph::VertexConnectivity>::array, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::VertexConnectivity>::name, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::Graph::VertexConnectivity>::set_name, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::Graph::VertexConnectivity> & (easy3d::Property<easy3d::Graph::VertexConnectivity>::*)(const class easy3d::Property<struct easy3d::Graph::VertexConnectivity> &)) &easy3d::Property<easy3d::Graph::VertexConnectivity>::operator=, "C++: easy3d::Property<easy3d::Graph::VertexConnectivity>::operator=(const class easy3d::Property<struct easy3d::Graph::VertexConnectivity> &) --> class easy3d::Property<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::Graph::VertexProperty file:easy3d/core/graph.h line:142
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexProperty<bool>, std::shared_ptr<easy3d::Graph::VertexProperty<bool>>, easy3d::Property<bool>> cl(enclosing_class, "VertexProperty_bool_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::VertexProperty<bool>(); } ) );
			cl.def( pybind11::init<class easy3d::Property<bool>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](easy3d::Graph::VertexProperty<bool> const &o){ return new easy3d::Graph::VertexProperty<bool>(o); } ) );
			cl.def("assign", (class easy3d::Graph::VertexProperty<bool> & (easy3d::Graph::VertexProperty<bool>::*)(const class easy3d::Graph::VertexProperty<bool> &)) &easy3d::Graph::VertexProperty<bool>::operator=, "C++: easy3d::Graph::VertexProperty<bool>::operator=(const class easy3d::Graph::VertexProperty<bool> &) --> class easy3d::Graph::VertexProperty<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::reset, "C++: easy3d::Property<bool>::reset() --> void");
			cl.def("data", (const bool * (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::data, "C++: easy3d::Property<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::vector, "C++: easy3d::Property<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::array, "C++: easy3d::Property<bool>::array() --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::name, "C++: easy3d::Property<bool>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<bool>::*)(const std::string &)) &easy3d::Property<bool>::set_name, "C++: easy3d::Property<bool>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<bool> & (easy3d::Property<bool>::*)(const class easy3d::Property<bool> &)) &easy3d::Property<bool>::operator=, "C++: easy3d::Property<bool>::operator=(const class easy3d::Property<bool> &) --> class easy3d::Property<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::Graph::VertexProperty file:easy3d/core/graph.h line:142
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>, std::shared_ptr<easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>>, PyCallBack_easy3d_Graph_VertexProperty_easy3d_Vec_3_float_t, easy3d::Property<easy3d::Vec<3, float>>> cl(enclosing_class, "VertexProperty_easy3d_Vec_3_float_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>(); }, [](){ return new PyCallBack_easy3d_Graph_VertexProperty_easy3d_Vec_3_float_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<class easy3d::Vec<3, float> >>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_Graph_VertexProperty_easy3d_Vec_3_float_t const &o){ return new PyCallBack_easy3d_Graph_VertexProperty_easy3d_Vec_3_float_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::Graph::VertexProperty<easy3d::Vec<3, float>> const &o){ return new easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>(o); } ) );
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::*)(struct easy3d::Graph::Vertex)) &easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::operator[], "C++: easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::operator[](struct easy3d::Graph::Vertex) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::Graph::VertexProperty<class easy3d::Vec<3, float> > & (easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::*)(const class easy3d::Graph::VertexProperty<class easy3d::Vec<3, float> > &)) &easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::operator=, "C++: easy3d::Graph::VertexProperty<easy3d::Vec<3, float>>::operator=(const class easy3d::Graph::VertexProperty<class easy3d::Vec<3, float> > &) --> class easy3d::Graph::VertexProperty<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::reset, "C++: easy3d::Property<easy3d::Vec<3, float>>::reset() --> void");
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::Property<easy3d::Vec<3, float>>::*)(unsigned long)) &easy3d::Property<easy3d::Vec<3, float>>::operator[], "C++: easy3d::Property<easy3d::Vec<3, float>>::operator[](unsigned long) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const class easy3d::Vec<3, float> * (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::data, "C++: easy3d::Property<easy3d::Vec<3, float>>::data() const --> const class easy3d::Vec<3, float> *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::vector, "C++: easy3d::Property<easy3d::Vec<3, float>>::vector() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::array, "C++: easy3d::Property<easy3d::Vec<3, float>>::array() --> class easy3d::PropertyArray<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::name, "C++: easy3d::Property<easy3d::Vec<3, float>>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::Vec<3, float>>::*)(const std::string &)) &easy3d::Property<easy3d::Vec<3, float>>::set_name, "C++: easy3d::Property<easy3d::Vec<3, float>>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)(const class easy3d::Property<class easy3d::Vec<3, float> > &)) &easy3d::Property<easy3d::Vec<3, float>>::operator=, "C++: easy3d::Property<easy3d::Vec<3, float>>::operator=(const class easy3d::Property<class easy3d::Vec<3, float> > &) --> class easy3d::Property<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::Graph::EdgeConnectivity>, std::shared_ptr<easy3d::Property<easy3d::Graph::EdgeConnectivity>>, PyCallBack_easy3d_Property_easy3d_Graph_EdgeConnectivity_t> cl(m, "Property_easy3d_Graph_EdgeConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::Graph::EdgeConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_Graph_EdgeConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_Graph_EdgeConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_Graph_EdgeConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::Graph::EdgeConnectivity> const &o){ return new easy3d::Property<easy3d::Graph::EdgeConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::reset, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::Graph::EdgeConnectivity & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)(unsigned long)) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator[], "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator[](unsigned long) --> struct easy3d::Graph::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::Graph::EdgeConnectivity * (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::data, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::data() const --> const struct easy3d::Graph::EdgeConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::vector, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::vector() --> class std::vector<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::array, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::name, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::set_name, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)(const class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> &)) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator=, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator=(const class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> &) --> class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::Graph::EdgeProperty file:easy3d/core/graph.h line:166
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>, std::shared_ptr<easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>>, PyCallBack_easy3d_Graph_EdgeProperty_easy3d_Graph_EdgeConnectivity_t, easy3d::Property<easy3d::Graph::EdgeConnectivity>> cl(enclosing_class, "EdgeProperty_easy3d_Graph_EdgeConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>(); }, [](){ return new PyCallBack_easy3d_Graph_EdgeProperty_easy3d_Graph_EdgeConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::Graph::EdgeConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_Graph_EdgeProperty_easy3d_Graph_EdgeConnectivity_t const &o){ return new PyCallBack_easy3d_Graph_EdgeProperty_easy3d_Graph_EdgeConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity> const &o){ return new easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::Graph::EdgeConnectivity & (easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::*)(struct easy3d::Graph::Edge)) &easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::operator[], "C++: easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::operator[](struct easy3d::Graph::Edge) --> struct easy3d::Graph::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("e"));
			cl.def("assign", (class easy3d::Graph::EdgeProperty<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::*)(const class easy3d::Graph::EdgeProperty<struct easy3d::Graph::EdgeConnectivity> &)) &easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::operator=, "C++: easy3d::Graph::EdgeProperty<easy3d::Graph::EdgeConnectivity>::operator=(const class easy3d::Graph::EdgeProperty<struct easy3d::Graph::EdgeConnectivity> &) --> class easy3d::Graph::EdgeProperty<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::reset, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::Graph::EdgeConnectivity & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)(unsigned long)) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator[], "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator[](unsigned long) --> struct easy3d::Graph::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::Graph::EdgeConnectivity * (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::data, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::data() const --> const struct easy3d::Graph::EdgeConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::vector, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::vector() --> class std::vector<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::array, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::name, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::set_name, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> & (easy3d::Property<easy3d::Graph::EdgeConnectivity>::*)(const class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> &)) &easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator=, "C++: easy3d::Property<easy3d::Graph::EdgeConnectivity>::operator=(const class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> &) --> class easy3d::Property<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::Graph::EdgeProperty file:easy3d/core/graph.h line:166
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::EdgeProperty<bool>, std::shared_ptr<easy3d::Graph::EdgeProperty<bool>>, easy3d::Property<bool>> cl(enclosing_class, "EdgeProperty_bool_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::EdgeProperty<bool>(); } ) );
			cl.def( pybind11::init<class easy3d::Property<bool>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](easy3d::Graph::EdgeProperty<bool> const &o){ return new easy3d::Graph::EdgeProperty<bool>(o); } ) );
			cl.def("assign", (class easy3d::Graph::EdgeProperty<bool> & (easy3d::Graph::EdgeProperty<bool>::*)(const class easy3d::Graph::EdgeProperty<bool> &)) &easy3d::Graph::EdgeProperty<bool>::operator=, "C++: easy3d::Graph::EdgeProperty<bool>::operator=(const class easy3d::Graph::EdgeProperty<bool> &) --> class easy3d::Graph::EdgeProperty<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::reset, "C++: easy3d::Property<bool>::reset() --> void");
			cl.def("data", (const bool * (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::data, "C++: easy3d::Property<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::vector, "C++: easy3d::Property<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::array, "C++: easy3d::Property<bool>::array() --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::name, "C++: easy3d::Property<bool>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<bool>::*)(const std::string &)) &easy3d::Property<bool>::set_name, "C++: easy3d::Property<bool>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<bool> & (easy3d::Property<bool>::*)(const class easy3d::Property<bool> &)) &easy3d::Property<bool>::operator=, "C++: easy3d::Property<bool>::operator=(const class easy3d::Property<bool> &) --> class easy3d::Property<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::Graph::VertexIterator file:easy3d/core/graph.h line:217
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexIterator, std::shared_ptr<easy3d::Graph::VertexIterator>> cl(enclosing_class, "VertexIterator", "this class iterates linearly over all vertices\n \n\n vertices_begin(), vertices_end()\n \n\n EdgeIterator");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::VertexIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::Graph::Vertex const & a0){ return new easy3d::Graph::VertexIterator(a0); } ), "doc" , pybind11::arg("v"));
			cl.def( pybind11::init<struct easy3d::Graph::Vertex, const class easy3d::Graph *>(), pybind11::arg("v"), pybind11::arg("g") );

			cl.def( pybind11::init( [](easy3d::Graph::VertexIterator const &o){ return new easy3d::Graph::VertexIterator(o); } ) );
			cl.def("dereference", (struct easy3d::Graph::Vertex (easy3d::Graph::VertexIterator::*)() const) &easy3d::Graph::VertexIterator::operator*, "get the vertex the iterator refers to\n\nC++: easy3d::Graph::VertexIterator::operator*() const --> struct easy3d::Graph::Vertex");
			cl.def("__eq__", (bool (easy3d::Graph::VertexIterator::*)(const class easy3d::Graph::VertexIterator &) const) &easy3d::Graph::VertexIterator::operator==, "are two iterators equal?\n\nC++: easy3d::Graph::VertexIterator::operator==(const class easy3d::Graph::VertexIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::Graph::VertexIterator::*)(const class easy3d::Graph::VertexIterator &) const) &easy3d::Graph::VertexIterator::operator!=, "are two iterators different?\n\nC++: easy3d::Graph::VertexIterator::operator!=(const class easy3d::Graph::VertexIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::Graph::VertexIterator & (easy3d::Graph::VertexIterator::*)()) &easy3d::Graph::VertexIterator::operator++, "pre-increment iterator\n\nC++: easy3d::Graph::VertexIterator::operator++() --> class easy3d::Graph::VertexIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::Graph::VertexIterator & (easy3d::Graph::VertexIterator::*)()) &easy3d::Graph::VertexIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::Graph::VertexIterator::operator--() --> class easy3d::Graph::VertexIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::Graph::EdgeIterator file:easy3d/core/graph.h line:269
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::EdgeIterator, std::shared_ptr<easy3d::Graph::EdgeIterator>> cl(enclosing_class, "EdgeIterator", "this class iterates linearly over all edges\n \n\n edges_begin(), edges_end()\n \n\n VertexIterator");
			cl.def( pybind11::init( [](){ return new easy3d::Graph::EdgeIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::Graph::Edge const & a0){ return new easy3d::Graph::EdgeIterator(a0); } ), "doc" , pybind11::arg("e"));
			cl.def( pybind11::init<struct easy3d::Graph::Edge, const class easy3d::Graph *>(), pybind11::arg("e"), pybind11::arg("g") );

			cl.def( pybind11::init( [](easy3d::Graph::EdgeIterator const &o){ return new easy3d::Graph::EdgeIterator(o); } ) );
			cl.def("dereference", (struct easy3d::Graph::Edge (easy3d::Graph::EdgeIterator::*)() const) &easy3d::Graph::EdgeIterator::operator*, "get the edge the iterator refers to\n\nC++: easy3d::Graph::EdgeIterator::operator*() const --> struct easy3d::Graph::Edge");
			cl.def("__eq__", (bool (easy3d::Graph::EdgeIterator::*)(const class easy3d::Graph::EdgeIterator &) const) &easy3d::Graph::EdgeIterator::operator==, "are two iterators equal?\n\nC++: easy3d::Graph::EdgeIterator::operator==(const class easy3d::Graph::EdgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::Graph::EdgeIterator::*)(const class easy3d::Graph::EdgeIterator &) const) &easy3d::Graph::EdgeIterator::operator!=, "are two iterators different?\n\nC++: easy3d::Graph::EdgeIterator::operator!=(const class easy3d::Graph::EdgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::Graph::EdgeIterator & (easy3d::Graph::EdgeIterator::*)()) &easy3d::Graph::EdgeIterator::operator++, "pre-increment iterator\n\nC++: easy3d::Graph::EdgeIterator::operator++() --> class easy3d::Graph::EdgeIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::Graph::EdgeIterator & (easy3d::Graph::EdgeIterator::*)()) &easy3d::Graph::EdgeIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::Graph::EdgeIterator::operator--() --> class easy3d::Graph::EdgeIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::Graph::VertexContainer file:easy3d/core/graph.h line:323
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexContainer, std::shared_ptr<easy3d::Graph::VertexContainer>> cl(enclosing_class, "VertexContainer", "this helper class is a container for iterating through all\n vertices using C++11 range-based for-loops.\n \n\n vertices()");
			cl.def( pybind11::init<class easy3d::Graph::VertexIterator, class easy3d::Graph::VertexIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::Graph::VertexContainer const &o){ return new easy3d::Graph::VertexContainer(o); } ) );
			cl.def("begin", (class easy3d::Graph::VertexIterator (easy3d::Graph::VertexContainer::*)() const) &easy3d::Graph::VertexContainer::begin, "C++: easy3d::Graph::VertexContainer::begin() const --> class easy3d::Graph::VertexIterator");
			cl.def("end", (class easy3d::Graph::VertexIterator (easy3d::Graph::VertexContainer::*)() const) &easy3d::Graph::VertexContainer::end, "C++: easy3d::Graph::VertexContainer::end() const --> class easy3d::Graph::VertexIterator");
		}

		{ // easy3d::Graph::EdgeContainer file:easy3d/core/graph.h line:336
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::EdgeContainer, std::shared_ptr<easy3d::Graph::EdgeContainer>> cl(enclosing_class, "EdgeContainer", "this helper class is a container for iterating through all\n edges using C++11 range-based for-loops.\n \n\n edges()");
			cl.def( pybind11::init<class easy3d::Graph::EdgeIterator, class easy3d::Graph::EdgeIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::Graph::EdgeContainer const &o){ return new easy3d::Graph::EdgeContainer(o); } ) );
			cl.def("begin", (class easy3d::Graph::EdgeIterator (easy3d::Graph::EdgeContainer::*)() const) &easy3d::Graph::EdgeContainer::begin, "C++: easy3d::Graph::EdgeContainer::begin() const --> class easy3d::Graph::EdgeIterator");
			cl.def("end", (class easy3d::Graph::EdgeIterator (easy3d::Graph::EdgeContainer::*)() const) &easy3d::Graph::EdgeContainer::end, "C++: easy3d::Graph::EdgeContainer::end() const --> class easy3d::Graph::EdgeIterator");
		}

		{ // easy3d::Graph::EdgeAroundVertexCirculator file:easy3d/core/graph.h line:353
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::EdgeAroundVertexCirculator, std::shared_ptr<easy3d::Graph::EdgeAroundVertexCirculator>> cl(enclosing_class, "EdgeAroundVertexCirculator", "this class circulates through all edges connected with a vertex.\n it also acts as a container-concept for C++11 range-based for loops.\n \n\n VertexAroundVertexCirculator, edges(Vertex)");
			cl.def( pybind11::init( [](const class easy3d::Graph * a0){ return new easy3d::Graph::EdgeAroundVertexCirculator(a0); } ), "doc" , pybind11::arg("g"));
			cl.def( pybind11::init<const class easy3d::Graph *, struct easy3d::Graph::Vertex>(), pybind11::arg("g"), pybind11::arg("v") );

			cl.def( pybind11::init( [](easy3d::Graph::EdgeAroundVertexCirculator const &o){ return new easy3d::Graph::EdgeAroundVertexCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::Graph::EdgeAroundVertexCirculator::*)(const class easy3d::Graph::EdgeAroundVertexCirculator &) const) &easy3d::Graph::EdgeAroundVertexCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::Graph::EdgeAroundVertexCirculator::operator==(const class easy3d::Graph::EdgeAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::Graph::EdgeAroundVertexCirculator::*)(const class easy3d::Graph::EdgeAroundVertexCirculator &) const) &easy3d::Graph::EdgeAroundVertexCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::Graph::EdgeAroundVertexCirculator::operator!=(const class easy3d::Graph::EdgeAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::Graph::EdgeAroundVertexCirculator & (easy3d::Graph::EdgeAroundVertexCirculator::*)()) &easy3d::Graph::EdgeAroundVertexCirculator::operator++, "pre-increment\n\nC++: easy3d::Graph::EdgeAroundVertexCirculator::operator++() --> class easy3d::Graph::EdgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::Graph::EdgeAroundVertexCirculator & (easy3d::Graph::EdgeAroundVertexCirculator::*)()) &easy3d::Graph::EdgeAroundVertexCirculator::operator--, "pre-decrement\n\nC++: easy3d::Graph::EdgeAroundVertexCirculator::operator--() --> class easy3d::Graph::EdgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::Graph::Edge (easy3d::Graph::EdgeAroundVertexCirculator::*)() const) &easy3d::Graph::EdgeAroundVertexCirculator::operator*, "get the edge the circulator refers to\n\nC++: easy3d::Graph::EdgeAroundVertexCirculator::operator*() const --> struct easy3d::Graph::Edge");
			cl.def("vertex", (struct easy3d::Graph::Vertex (easy3d::Graph::EdgeAroundVertexCirculator::*)() const) &easy3d::Graph::EdgeAroundVertexCirculator::vertex, "return current vertex\n\nC++: easy3d::Graph::EdgeAroundVertexCirculator::vertex() const --> struct easy3d::Graph::Vertex");
			cl.def("begin", (class easy3d::Graph::EdgeAroundVertexCirculator & (easy3d::Graph::EdgeAroundVertexCirculator::*)()) &easy3d::Graph::EdgeAroundVertexCirculator::begin, "C++: easy3d::Graph::EdgeAroundVertexCirculator::begin() --> class easy3d::Graph::EdgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::Graph::EdgeAroundVertexCirculator & (easy3d::Graph::EdgeAroundVertexCirculator::*)()) &easy3d::Graph::EdgeAroundVertexCirculator::end, "C++: easy3d::Graph::EdgeAroundVertexCirculator::end() --> class easy3d::Graph::EdgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::Graph::VertexAroundVertexCirculator file:easy3d/core/graph.h line:428
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::Graph::VertexAroundVertexCirculator, std::shared_ptr<easy3d::Graph::VertexAroundVertexCirculator>> cl(enclosing_class, "VertexAroundVertexCirculator", "this class circulates through all one-ring neighbors of a vertex.\n it also acts as a container-concept for C++11 range-based for loops.\n \n\n EdgeAroundVertexCirculator, vertices(Vertex)");
			cl.def( pybind11::init( [](const class easy3d::Graph * a0){ return new easy3d::Graph::VertexAroundVertexCirculator(a0); } ), "doc" , pybind11::arg("g"));
			cl.def( pybind11::init<const class easy3d::Graph *, struct easy3d::Graph::Vertex>(), pybind11::arg("g"), pybind11::arg("v") );

			cl.def( pybind11::init( [](easy3d::Graph::VertexAroundVertexCirculator const &o){ return new easy3d::Graph::VertexAroundVertexCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::Graph::VertexAroundVertexCirculator::*)(const class easy3d::Graph::VertexAroundVertexCirculator &) const) &easy3d::Graph::VertexAroundVertexCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::Graph::VertexAroundVertexCirculator::operator==(const class easy3d::Graph::VertexAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::Graph::VertexAroundVertexCirculator::*)(const class easy3d::Graph::VertexAroundVertexCirculator &) const) &easy3d::Graph::VertexAroundVertexCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::Graph::VertexAroundVertexCirculator::operator!=(const class easy3d::Graph::VertexAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::Graph::VertexAroundVertexCirculator & (easy3d::Graph::VertexAroundVertexCirculator::*)()) &easy3d::Graph::VertexAroundVertexCirculator::operator++, "pre-increment\n\nC++: easy3d::Graph::VertexAroundVertexCirculator::operator++() --> class easy3d::Graph::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::Graph::VertexAroundVertexCirculator & (easy3d::Graph::VertexAroundVertexCirculator::*)()) &easy3d::Graph::VertexAroundVertexCirculator::operator--, "pre-decrement\n\nC++: easy3d::Graph::VertexAroundVertexCirculator::operator--() --> class easy3d::Graph::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::Graph::Vertex (easy3d::Graph::VertexAroundVertexCirculator::*)() const) &easy3d::Graph::VertexAroundVertexCirculator::operator*, "get the vertex the circulator refers to\n\nC++: easy3d::Graph::VertexAroundVertexCirculator::operator*() const --> struct easy3d::Graph::Vertex");
			cl.def("vertex", (struct easy3d::Graph::Vertex (easy3d::Graph::VertexAroundVertexCirculator::*)() const) &easy3d::Graph::VertexAroundVertexCirculator::vertex, "return current vertex\n\nC++: easy3d::Graph::VertexAroundVertexCirculator::vertex() const --> struct easy3d::Graph::Vertex");
			cl.def("begin", (class easy3d::Graph::VertexAroundVertexCirculator & (easy3d::Graph::VertexAroundVertexCirculator::*)()) &easy3d::Graph::VertexAroundVertexCirculator::begin, "C++: easy3d::Graph::VertexAroundVertexCirculator::begin() --> class easy3d::Graph::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::Graph::VertexAroundVertexCirculator & (easy3d::Graph::VertexAroundVertexCirculator::*)()) &easy3d::Graph::VertexAroundVertexCirculator::end, "C++: easy3d::Graph::VertexAroundVertexCirculator::end() --> class easy3d::Graph::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
		}
	}

    { // easy3d::PropertyArray file:easy3d/core/property.h line:110
        pybind11::class_<easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_Graph_VertexConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_Graph_VertexConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_Graph_VertexConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::Graph::VertexConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_Graph_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_Graph_VertexConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::Graph::VertexConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::Graph::VertexConnectivity * (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::data() const --> const struct easy3d::Graph::VertexConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::Graph::VertexConnectivity> & (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::vector() --> class std::vector<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::Graph::VertexConnectivity & (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::operator[](unsigned long) --> struct easy3d::Graph::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> & (easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> &)) &easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::Graph::VertexConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::Graph::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        cl.def("reserve", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reserve, "Reserve memory for n elements.\n\nC++: easy3d::BasePropertyArray::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::resize, "Resize storage to hold n elements.\n\nC++: easy3d::BasePropertyArray::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("shrink_to_fit", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::shrink_to_fit, "Free unused memory.\n\nC++: easy3d::BasePropertyArray::shrink_to_fit() --> void");
        cl.def("push_back", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::push_back, "Extend the number of elements by one.\n\nC++: easy3d::BasePropertyArray::push_back() --> void");
        cl.def("reset", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reset, "Reset element to default value\n\nC++: easy3d::BasePropertyArray::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::transfer, "Copy the entire properties from \n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::BasePropertyArray::transfer, "Copy the property[from] of  to this->property[to].\n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("swap", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::swap, "Let two elements swap their storage place.\n\nC++: easy3d::BasePropertyArray::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::copy, "Let copy 'from' -> 'to'.\n\nC++: easy3d::BasePropertyArray::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::clone, "Return a deep copy of self.\n\nC++: easy3d::BasePropertyArray::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::empty_clone, "Return a empty copy of self.\n\nC++: easy3d::BasePropertyArray::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::type, "Return the type_info of the property\n\nC++: easy3d::BasePropertyArray::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::name, "Return the name of the property\n\nC++: easy3d::BasePropertyArray::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::BasePropertyArray::*)(const std::string &)) &easy3d::BasePropertyArray::set_name, "Set the name of the property\n\nC++: easy3d::BasePropertyArray::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("is_same", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &) const) &easy3d::BasePropertyArray::is_same, "Test if two properties are the same.\n \n\n true only if their names and types are both identical.\n\nC++: easy3d::BasePropertyArray::is_same(const class easy3d::BasePropertyArray &) const --> bool", pybind11::arg("other"));
        cl.def("assign", (class easy3d::BasePropertyArray & (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::operator=, "C++: easy3d::BasePropertyArray::operator=(const class easy3d::BasePropertyArray &) --> class easy3d::BasePropertyArray &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // easy3d::PropertyArray file:easy3d/core/property.h line:110
        pybind11::class_<easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_Graph_EdgeConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_Graph_EdgeConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_Graph_EdgeConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::Graph::EdgeConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_Graph_EdgeConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_Graph_EdgeConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::Graph::EdgeConnectivity * (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::data() const --> const struct easy3d::Graph::EdgeConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::Graph::EdgeConnectivity> & (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::vector() --> class std::vector<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::Graph::EdgeConnectivity & (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::operator[](unsigned long) --> struct easy3d::Graph::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> & (easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> &)) &easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::Graph::EdgeConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::Graph::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        cl.def("reserve", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reserve, "Reserve memory for n elements.\n\nC++: easy3d::BasePropertyArray::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::resize, "Resize storage to hold n elements.\n\nC++: easy3d::BasePropertyArray::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("shrink_to_fit", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::shrink_to_fit, "Free unused memory.\n\nC++: easy3d::BasePropertyArray::shrink_to_fit() --> void");
        cl.def("push_back", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::push_back, "Extend the number of elements by one.\n\nC++: easy3d::BasePropertyArray::push_back() --> void");
        cl.def("reset", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reset, "Reset element to default value\n\nC++: easy3d::BasePropertyArray::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::transfer, "Copy the entire properties from \n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::BasePropertyArray::transfer, "Copy the property[from] of  to this->property[to].\n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("swap", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::swap, "Let two elements swap their storage place.\n\nC++: easy3d::BasePropertyArray::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::copy, "Let copy 'from' -> 'to'.\n\nC++: easy3d::BasePropertyArray::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::clone, "Return a deep copy of self.\n\nC++: easy3d::BasePropertyArray::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::empty_clone, "Return a empty copy of self.\n\nC++: easy3d::BasePropertyArray::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::type, "Return the type_info of the property\n\nC++: easy3d::BasePropertyArray::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::name, "Return the name of the property\n\nC++: easy3d::BasePropertyArray::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::BasePropertyArray::*)(const std::string &)) &easy3d::BasePropertyArray::set_name, "Set the name of the property\n\nC++: easy3d::BasePropertyArray::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("is_same", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &) const) &easy3d::BasePropertyArray::is_same, "Test if two properties are the same.\n \n\n true only if their names and types are both identical.\n\nC++: easy3d::BasePropertyArray::is_same(const class easy3d::BasePropertyArray &) const --> bool", pybind11::arg("other"));
        cl.def("assign", (class easy3d::BasePropertyArray & (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::operator=, "C++: easy3d::BasePropertyArray::operator=(const class easy3d::BasePropertyArray &) --> class easy3d::BasePropertyArray &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
}
