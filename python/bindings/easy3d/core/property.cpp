#include <easy3d/core/property.h>
#include <easy3d/core/types.h>

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


// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_float_t : public easy3d::Property<float> {
    using easy3d::Property<float>::Property;

    float & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<float> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<float &>::value) {
                static pybind11::detail::override_caster_t<float &> caster;
                return pybind11::detail::cast_ref<float &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<float &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_double_t : public easy3d::Property<double> {
    using easy3d::Property<double>::Property;

    double & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<double> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<double &>::value) {
                static pybind11::detail::override_caster_t<double &> caster;
                return pybind11::detail::cast_ref<double &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<double &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_int_t : public easy3d::Property<int> {
    using easy3d::Property<int>::Property;

    int & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<int> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int &>::value) {
                static pybind11::detail::override_caster_t<int &> caster;
                return pybind11::detail::cast_ref<int &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<int &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};


// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_Vec_3_float_t : public easy3d::Property<easy3d::Vec<3, float>> {
    using easy3d::Property<easy3d::Vec<3, float>>::Property;

    using _binder_ret_0 = easy3d::Vec<3, float> &;
    _binder_ret_0 operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::Vec<3, float>> *>(this), "__getitem__");
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


// easy3d::BasePropertyArray file:easy3d/core/property.h line:40
struct PyCallBack_easy3d_BasePropertyArray : public easy3d::BasePropertyArray {
	using easy3d::BasePropertyArray::BasePropertyArray;

	void reserve(size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "reserve");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::reserve\"");
	}
	void resize(size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "resize");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::resize\"");
	}
	void shrink_to_fit() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "shrink_to_fit");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::shrink_to_fit\"");
	}
	void push_back() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "push_back");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::push_back\"");
	}
	void reset(size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "reset");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::reset\"");
	}
	bool transfer(const class easy3d::BasePropertyArray & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "transfer");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::transfer\"");
	}
	bool transfer(const class easy3d::BasePropertyArray & a0, std::size_t a1, std::size_t a2) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "transfer");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::transfer\"");
	}
	void swap(size_t a0, size_t a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "swap");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::swap\"");
	}
	void copy(size_t a0, size_t a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "copy");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::copy\"");
	}
	class easy3d::BasePropertyArray * clone() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "clone");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::BasePropertyArray *>::value) {
				static pybind11::detail::override_caster_t<class easy3d::BasePropertyArray *> caster;
				return pybind11::detail::cast_ref<class easy3d::BasePropertyArray *>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<class easy3d::BasePropertyArray *>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::clone\"");
	}
	class easy3d::BasePropertyArray * empty_clone() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "empty_clone");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::BasePropertyArray *>::value) {
				static pybind11::detail::override_caster_t<class easy3d::BasePropertyArray *> caster;
				return pybind11::detail::cast_ref<class easy3d::BasePropertyArray *>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<class easy3d::BasePropertyArray *>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::empty_clone\"");
	}
	const class std::type_info & type() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::BasePropertyArray *>(this), "type");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<const class std::type_info &>::value) {
				static pybind11::detail::override_caster_t<const class std::type_info &> caster;
				return pybind11::detail::cast_ref<const class std::type_info &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<const class std::type_info &>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"BasePropertyArray::type\"");
	}
};

// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_bool_t : public easy3d::PropertyArray<bool> {
    using easy3d::PropertyArray<bool>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "reserve");
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
    void resize(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "push_back");
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
    void reset(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "shrink_to_fit");
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
    void swap(size_t a0, size_t a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "swap");
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
    void copy(size_t a0, size_t a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<bool> *>(this), "type");
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


// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_std_vector_easy3d_Vec_3_float_t : public easy3d::Property<std::vector<easy3d::Vec<3, float> >> {
    using easy3d::Property<std::vector<easy3d::Vec<3, float> >>::Property;

    using _binder_ret_0 = std::vector<class easy3d::Vec<3, float> > &;
    _binder_ret_0 operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<std::vector<easy3d::Vec<3, float> >> *>(this), "__getitem__");
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

// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_easy3d_Vec_3_float_t : public easy3d::PropertyArray<easy3d::Vec<3, float>> {
    using easy3d::PropertyArray<easy3d::Vec<3, float>>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "reserve");
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
    void resize(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "push_back");
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
    void reset(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "shrink_to_fit");
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
    void swap(size_t a0, size_t a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "swap");
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
    void copy(size_t a0, size_t a1) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::Vec<3, float>> *>(this), "type");
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

void bind_easy3d_core_property(pybind11::module_& m)
{
    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<bool>, std::shared_ptr<easy3d::Property<bool>>> cl(m, "Property_bool_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<bool>(); } ), "doc" );
        cl.def( pybind11::init<class easy3d::PropertyArray<bool> *>(), pybind11::arg("p") );

        cl.def( pybind11::init( [](easy3d::Property<bool> const &o){ return new easy3d::Property<bool>(o); } ) );
        cl.def("reset", (void (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::reset, "C++: easy3d::Property<bool>::reset() --> void");
        cl.def("data", (const bool * (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::data, "C++: easy3d::Property<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::vector, "C++: easy3d::Property<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::array, "C++: easy3d::Property<bool>::array() --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::name, "C++: easy3d::Property<bool>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<bool>::*)(const std::string &)) &easy3d::Property<bool>::set_name, "C++: easy3d::Property<bool>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<bool> & (easy3d::Property<bool>::*)(const class easy3d::Property<bool> &)) &easy3d::Property<bool>::operator=, "C++: easy3d::Property<bool>::operator=(const class easy3d::Property<bool> &) --> class easy3d::Property<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<easy3d::Vec<3, float>>, std::shared_ptr<easy3d::Property<easy3d::Vec<3, float>>>, PyCallBack_easy3d_Property_easy3d_Vec_3_float_t> cl(m, "Property_easy3d_Vec_3_float_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::Vec<3, float>>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_Vec_3_float_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<class easy3d::Vec<3, float> > *>(), pybind11::arg("p") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_Vec_3_float_t const &o){ return new PyCallBack_easy3d_Property_easy3d_Vec_3_float_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::Property<easy3d::Vec<3, float>> const &o){ return new easy3d::Property<easy3d::Vec<3, float>>(o); } ) );
        cl.def("reset", (void (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::reset, "C++: easy3d::Property<easy3d::Vec<3, float>>::reset() --> void");
        cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::Property<easy3d::Vec<3, float>>::*)(size_t)) &easy3d::Property<easy3d::Vec<3, float>>::operator[], "C++: easy3d::Property<easy3d::Vec<3, float>>::operator[](size_t) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const class easy3d::Vec<3, float> * (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::data, "C++: easy3d::Property<easy3d::Vec<3, float>>::data() const --> const class easy3d::Vec<3, float> *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::vector, "C++: easy3d::Property<easy3d::Vec<3, float>>::vector() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::array, "C++: easy3d::Property<easy3d::Vec<3, float>>::array() --> class easy3d::PropertyArray<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::name, "C++: easy3d::Property<easy3d::Vec<3, float>>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<easy3d::Vec<3, float>>::*)(const std::string &)) &easy3d::Property<easy3d::Vec<3, float>>::set_name, "C++: easy3d::Property<easy3d::Vec<3, float>>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)(const class easy3d::Property<class easy3d::Vec<3, float> > &)) &easy3d::Property<easy3d::Vec<3, float>>::operator=, "C++: easy3d::Property<easy3d::Vec<3, float>>::operator=(const class easy3d::Property<class easy3d::Vec<3, float> > &) --> class easy3d::Property<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<float>, std::shared_ptr<easy3d::Property<float>>, PyCallBack_easy3d_Property_float_t> cl(m, "Property_float_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<float>(); }, [](){ return new PyCallBack_easy3d_Property_float_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<float> *>(), pybind11::arg("p") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_Property_float_t const &o){ return new PyCallBack_easy3d_Property_float_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::Property<float> const &o){ return new easy3d::Property<float>(o); } ) );
        cl.def("reset", (void (easy3d::Property<float>::*)()) &easy3d::Property<float>::reset, "C++: easy3d::Property<float>::reset() --> void");
        cl.def("__getitem__", (float & (easy3d::Property<float>::*)(size_t)) &easy3d::Property<float>::operator[], "C++: easy3d::Property<float>::operator[](size_t) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const float * (easy3d::Property<float>::*)() const) &easy3d::Property<float>::data, "C++: easy3d::Property<float>::data() const --> const float *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<float> & (easy3d::Property<float>::*)()) &easy3d::Property<float>::vector, "C++: easy3d::Property<float>::vector() --> class std::vector<float> &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<float> & (easy3d::Property<float>::*)()) &easy3d::Property<float>::array, "C++: easy3d::Property<float>::array() --> class easy3d::PropertyArray<float> &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<float>::*)() const) &easy3d::Property<float>::name, "C++: easy3d::Property<float>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<float>::*)(const std::string &)) &easy3d::Property<float>::set_name, "C++: easy3d::Property<float>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<float> & (easy3d::Property<float>::*)(const class easy3d::Property<float> &)) &easy3d::Property<float>::operator=, "C++: easy3d::Property<float>::operator=(const class easy3d::Property<float> &) --> class easy3d::Property<float> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<double>, std::shared_ptr<easy3d::Property<double>>, PyCallBack_easy3d_Property_double_t> cl(m, "Property_double_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<double>(); }, [](){ return new PyCallBack_easy3d_Property_double_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<double> *>(), pybind11::arg("p") );

        cl.def("reset", (void (easy3d::Property<double>::*)()) &easy3d::Property<double>::reset, "C++: easy3d::Property<double>::reset() --> void");
        cl.def("__getitem__", (double & (easy3d::Property<double>::*)(size_t)) &easy3d::Property<double>::operator[], "C++: easy3d::Property<double>::operator[](size_t) --> double &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const double * (easy3d::Property<double>::*)() const) &easy3d::Property<double>::data, "C++: easy3d::Property<double>::data() const --> const double *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<double> & (easy3d::Property<double>::*)()) &easy3d::Property<double>::vector, "C++: easy3d::Property<double>::vector() --> class std::vector<double> &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<double> & (easy3d::Property<double>::*)()) &easy3d::Property<double>::array, "C++: easy3d::Property<double>::array() --> class easy3d::PropertyArray<double> &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<double>::*)() const) &easy3d::Property<double>::name, "C++: easy3d::Property<double>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<double>::*)(const std::string &)) &easy3d::Property<double>::set_name, "C++: easy3d::Property<double>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<double> & (easy3d::Property<double>::*)(const class easy3d::Property<double> &)) &easy3d::Property<double>::operator=, "C++: easy3d::Property<double>::operator=(const class easy3d::Property<double> &) --> class easy3d::Property<double> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<int>, std::shared_ptr<easy3d::Property<int>>, PyCallBack_easy3d_Property_int_t> cl(m, "Property_int_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<int>(); }, [](){ return new PyCallBack_easy3d_Property_int_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<int> *>(), pybind11::arg("p") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_Property_int_t const &o){ return new PyCallBack_easy3d_Property_int_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::Property<int> const &o){ return new easy3d::Property<int>(o); } ) );
        cl.def("reset", (void (easy3d::Property<int>::*)()) &easy3d::Property<int>::reset, "C++: easy3d::Property<int>::reset() --> void");
        cl.def("__getitem__", (int & (easy3d::Property<int>::*)(size_t)) &easy3d::Property<int>::operator[], "C++: easy3d::Property<int>::operator[](unsigned long) --> int &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const int * (easy3d::Property<int>::*)() const) &easy3d::Property<int>::data, "C++: easy3d::Property<int>::data() const --> const int *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<int> & (easy3d::Property<int>::*)()) &easy3d::Property<int>::vector, "C++: easy3d::Property<int>::vector() --> class std::vector<int> &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<int> & (easy3d::Property<int>::*)()) &easy3d::Property<int>::array, "C++: easy3d::Property<int>::array() --> class easy3d::PropertyArray<int> &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<int>::*)() const) &easy3d::Property<int>::name, "C++: easy3d::Property<int>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<int>::*)(const std::string &)) &easy3d::Property<int>::set_name, "C++: easy3d::Property<int>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<int> & (easy3d::Property<int>::*)(const class easy3d::Property<int> &)) &easy3d::Property<int>::operator=, "C++: easy3d::Property<int>::operator=(const class easy3d::Property<int> &) --> class easy3d::Property<int> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

	{ // easy3d::PropertyContainer file:easy3d/core/property.h line:340
		pybind11::class_<easy3d::PropertyContainer, std::shared_ptr<easy3d::PropertyContainer>> cl(m, "PropertyContainer", "Implementation of generic property container.\n \n");
		cl.def( pybind11::init( [](){ return new easy3d::PropertyContainer(); } ) );
		cl.def( pybind11::init( [](easy3d::PropertyContainer const &o){ return new easy3d::PropertyContainer(o); } ) );
		cl.def("assign", (class easy3d::PropertyContainer & (easy3d::PropertyContainer::*)(const class easy3d::PropertyContainer &)) &easy3d::PropertyContainer::operator=, "C++: easy3d::PropertyContainer::operator=(const class easy3d::PropertyContainer &) --> class easy3d::PropertyContainer &", pybind11::return_value_policy::automatic, pybind11::arg("_rhs"));
		cl.def("transfer", (void (easy3d::PropertyContainer::*)(const class easy3d::PropertyContainer &) const) &easy3d::PropertyContainer::transfer, "C++: easy3d::PropertyContainer::transfer(const class easy3d::PropertyContainer &) --> void", pybind11::arg("_rhs"));
		cl.def("copy_properties", (void (easy3d::PropertyContainer::*)(const class easy3d::PropertyContainer &)) &easy3d::PropertyContainer::copy_properties, "C++: easy3d::PropertyContainer::copy_properties(const class easy3d::PropertyContainer &) --> void", pybind11::arg("_rhs"));
		cl.def("transfer", (bool (easy3d::PropertyContainer::*)(const class easy3d::PropertyContainer &, std::size_t, std::size_t) const) &easy3d::PropertyContainer::transfer, "C++: easy3d::PropertyContainer::transfer(const class easy3d::PropertyContainer &, std::size_t, std::size_t) --> bool", pybind11::arg("_rhs"), pybind11::arg("from"), pybind11::arg("to"));
		cl.def("size", (unsigned long (easy3d::PropertyContainer::*)() const) &easy3d::PropertyContainer::size, "C++: easy3d::PropertyContainer::size() const --> unsigned long");
		cl.def("n_properties", (unsigned long (easy3d::PropertyContainer::*)() const) &easy3d::PropertyContainer::n_properties, "C++: easy3d::PropertyContainer::n_properties() const --> unsigned long");
		cl.def("properties", (class std::vector<std::string> (easy3d::PropertyContainer::*)() const) &easy3d::PropertyContainer::properties, "C++: easy3d::PropertyContainer::properties() const --> class std::vector<std::string>");
		cl.def("get_type", (const class std::type_info & (easy3d::PropertyContainer::*)(const std::string &) const) &easy3d::PropertyContainer::get_type, "C++: easy3d::PropertyContainer::get_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("remove", (bool (easy3d::PropertyContainer::*)(const std::string &)) &easy3d::PropertyContainer::remove, "C++: easy3d::PropertyContainer::remove(const std::string &) --> bool", pybind11::arg("name"));
		cl.def("rename", (bool (easy3d::PropertyContainer::*)(const std::string &, const std::string &)) &easy3d::PropertyContainer::rename, "C++: easy3d::PropertyContainer::rename(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("clear", (void (easy3d::PropertyContainer::*)()) &easy3d::PropertyContainer::clear, "C++: easy3d::PropertyContainer::clear() --> void");
		cl.def("reserve", (void (easy3d::PropertyContainer::*)(unsigned long) const) &easy3d::PropertyContainer::reserve, "C++: easy3d::PropertyContainer::reserve(unsigned long) const --> void", pybind11::arg("n"));
		cl.def("resize", (void (easy3d::PropertyContainer::*)(unsigned long)) &easy3d::PropertyContainer::resize, "C++: easy3d::PropertyContainer::resize(unsigned long) --> void", pybind11::arg("n"));
		cl.def("resize_property_array", (void (easy3d::PropertyContainer::*)(unsigned long)) &easy3d::PropertyContainer::resize_property_array, "C++: easy3d::PropertyContainer::resize_property_array(unsigned long) --> void", pybind11::arg("n"));
		cl.def("shrink_to_fit", (void (easy3d::PropertyContainer::*)() const) &easy3d::PropertyContainer::shrink_to_fit, "C++: easy3d::PropertyContainer::shrink_to_fit() const --> void");
		cl.def("push_back", (void (easy3d::PropertyContainer::*)()) &easy3d::PropertyContainer::push_back, "C++: easy3d::PropertyContainer::push_back() --> void");
		cl.def("reset", (void (easy3d::PropertyContainer::*)(unsigned long)) &easy3d::PropertyContainer::reset, "C++: easy3d::PropertyContainer::reset(unsigned long) --> void", pybind11::arg("idx"));
		cl.def("swap", (void (easy3d::PropertyContainer::*)(size_t, size_t) const) &easy3d::PropertyContainer::swap, "C++: easy3d::PropertyContainer::swap(size_t, size_t) const --> void", pybind11::arg("i0"), pybind11::arg("i1"));
		cl.def("swap", (void (easy3d::PropertyContainer::*)(class easy3d::PropertyContainer &)) &easy3d::PropertyContainer::swap, "C++: easy3d::PropertyContainer::swap(class easy3d::PropertyContainer &) --> void", pybind11::arg("other"));
		cl.def("copy", (void (easy3d::PropertyContainer::*)(unsigned long, unsigned long) const) &easy3d::PropertyContainer::copy, "C++: easy3d::PropertyContainer::copy(unsigned long, unsigned long) const --> void", pybind11::arg("from"), pybind11::arg("to"));
		cl.def("arrays", (class std::vector<class easy3d::BasePropertyArray *> & (easy3d::PropertyContainer::*)()) &easy3d::PropertyContainer::arrays, "C++: easy3d::PropertyContainer::arrays() --> class std::vector<class easy3d::BasePropertyArray *> &", pybind11::return_value_policy::automatic);
	}

    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<std::vector<easy3d::Vec<3, float> >>, std::shared_ptr<easy3d::Property<std::vector<easy3d::Vec<3, float> >>>, PyCallBack_easy3d_Property_std_vector_easy3d_Vec_3_float_t> cl(m, "Property_std_vector_easy3d_Vec_3_float_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<std::vector<easy3d::Vec<3, float> >>(); }, [](){ return new PyCallBack_easy3d_Property_std_vector_easy3d_Vec_3_float_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<class std::vector<class easy3d::Vec<3, float> > > *>(), pybind11::arg("p") );

        cl.def("reset", (void (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)()) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::reset, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::reset() --> void");
        cl.def("__getitem__", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)(size_t)) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::operator[], "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::operator[](size_t) --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const class std::vector<class easy3d::Vec<3, float> > * (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)() const) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::data, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::data() const --> const class std::vector<class easy3d::Vec<3, float> > *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<class std::vector<class easy3d::Vec<3, float> > > & (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)()) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::vector, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::vector() --> class std::vector<class std::vector<class easy3d::Vec<3, float> > > &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<class std::vector<class easy3d::Vec<3, float> > > & (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)()) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::array, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::array() --> class easy3d::PropertyArray<class std::vector<class easy3d::Vec<3, float> > > &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)() const) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::name, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)(const std::string &)) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::set_name, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<class std::vector<class easy3d::Vec<3, float> > > & (easy3d::Property<std::vector<easy3d::Vec<3, float> >>::*)(const class easy3d::Property<class std::vector<class easy3d::Vec<3, float> > > &)) &easy3d::Property<std::vector<easy3d::Vec<3, float>>>::operator=, "C++: easy3d::Property<std::vector<easy3d::Vec<3, float>>>::operator=(const class easy3d::Property<class std::vector<class easy3d::Vec<3, float> > > &) --> class easy3d::Property<class std::vector<class easy3d::Vec<3, float> > > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

    { // easy3d::BasePropertyArray file:easy3d/core/property.h line:40
        pybind11::class_<easy3d::BasePropertyArray, std::shared_ptr<easy3d::BasePropertyArray>, PyCallBack_easy3d_BasePropertyArray> cl(m, "BasePropertyArray", "Base class for a property array.\n \n");
        cl.def( pybind11::init<const std::string &>(), pybind11::arg("name") );

        cl.def(pybind11::init<PyCallBack_easy3d_BasePropertyArray const &>());
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
        pybind11::class_<easy3d::PropertyArray<easy3d::Vec<3, float>>, std::shared_ptr<easy3d::PropertyArray<easy3d::Vec<3, float>>>, PyCallBack_easy3d_PropertyArray_easy3d_Vec_3_float_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_Vec_3_float_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::Vec<3, float>>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_Vec_3_float_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, class easy3d::Vec<3, float>>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_Vec_3_float_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_Vec_3_float_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::Vec<3, float>> const &o){ return new easy3d::PropertyArray<easy3d::Vec<3, float>>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::reserve, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::resize, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)()) &easy3d::PropertyArray<easy3d::Vec<3, float>>::push_back, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::reset, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::transfer, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::transfer, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)()) &easy3d::PropertyArray<easy3d::Vec<3, float>>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::swap, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::copy, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)() const) &easy3d::PropertyArray<easy3d::Vec<3, float>>::clone, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)() const) &easy3d::PropertyArray<easy3d::Vec<3, float>>::empty_clone, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)() const) &easy3d::PropertyArray<easy3d::Vec<3, float>>::type, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const class easy3d::Vec<3, float> * (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)() const) &easy3d::PropertyArray<easy3d::Vec<3, float>>::data, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::data() const --> const class easy3d::Vec<3, float> *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)()) &easy3d::PropertyArray<easy3d::Vec<3, float>>::vector, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::vector() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(size_t)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::operator[], "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::operator[](size_t) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<class easy3d::Vec<3, float> > & (easy3d::PropertyArray<easy3d::Vec<3, float>>::*)(const class easy3d::PropertyArray<class easy3d::Vec<3, float> > &)) &easy3d::PropertyArray<easy3d::Vec<3, float>>::operator=, "C++: easy3d::PropertyArray<easy3d::Vec<3, float>>::operator=(const class easy3d::PropertyArray<class easy3d::Vec<3, float> > &) --> class easy3d::PropertyArray<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
        pybind11::class_<easy3d::PropertyArray<bool>, std::shared_ptr<easy3d::PropertyArray<bool>>, PyCallBack_easy3d_PropertyArray_bool_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_bool_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<bool>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_bool_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, bool>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_bool_t const &o){ return new PyCallBack_easy3d_PropertyArray_bool_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<bool> const &o){ return new easy3d::PropertyArray<bool>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<bool>::*)(unsigned long)) &easy3d::PropertyArray<bool>::reserve, "C++: easy3d::PropertyArray<bool>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<bool>::*)(unsigned long)) &easy3d::PropertyArray<bool>::resize, "C++: easy3d::PropertyArray<bool>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<bool>::*)()) &easy3d::PropertyArray<bool>::push_back, "C++: easy3d::PropertyArray<bool>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<bool>::*)(unsigned long)) &easy3d::PropertyArray<bool>::reset, "C++: easy3d::PropertyArray<bool>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<bool>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<bool>::transfer, "C++: easy3d::PropertyArray<bool>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<bool>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<bool>::transfer, "C++: easy3d::PropertyArray<bool>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<bool>::*)()) &easy3d::PropertyArray<bool>::shrink_to_fit, "C++: easy3d::PropertyArray<bool>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<bool>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<bool>::swap, "C++: easy3d::PropertyArray<bool>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<bool>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<bool>::copy, "C++: easy3d::PropertyArray<bool>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::clone, "C++: easy3d::PropertyArray<bool>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::empty_clone, "C++: easy3d::PropertyArray<bool>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::type, "C++: easy3d::PropertyArray<bool>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const bool * (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::data, "C++: easy3d::PropertyArray<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<bool> & (easy3d::PropertyArray<bool>::*)()) &easy3d::PropertyArray<bool>::vector, "C++: easy3d::PropertyArray<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
        cl.def("assign", (class easy3d::PropertyArray<bool> & (easy3d::PropertyArray<bool>::*)(const class easy3d::PropertyArray<bool> &)) &easy3d::PropertyArray<bool>::operator=, "C++: easy3d::PropertyArray<bool>::operator=(const class easy3d::PropertyArray<bool> &) --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
