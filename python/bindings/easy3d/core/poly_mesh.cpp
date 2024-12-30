#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/property.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/renderer.h>

#include <iterator>
#include <memory>
#include <set>
#include <typeinfo>
#include <utility>
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
struct PyCallBack_easy3d_Property_easy3d_PolyMesh_HalfFaceConnectivity_t : public easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity> {
    using easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::Property;

    struct easy3d::PolyMesh::HalfFaceConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::HalfFaceConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::HalfFaceConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::PolyMesh::HalfFaceConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::PolyMesh::HalfFaceConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_PolyMesh_CellConnectivity_t : public easy3d::Property<easy3d::PolyMesh::CellConnectivity> {
    using easy3d::Property<easy3d::PolyMesh::CellConnectivity>::Property;

    struct easy3d::PolyMesh::CellConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::PolyMesh::CellConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::CellConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::CellConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::PolyMesh::CellConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::PolyMesh::CellConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};


// easy3d::PolyMesh file:easy3d/core/poly_mesh.h line:49
struct PyCallBack_easy3d_PolyMesh : public easy3d::PolyMesh {
	using easy3d::PolyMesh::PolyMesh;

	void property_stats(std::ostream & a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh *>(this), "property_stats");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return PolyMesh::property_stats(a0);
	}
	using _binder_ret_0 = std::vector<class easy3d::Vec<3, float> > &;
	_binder_ret_0 points() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh *>(this), "points");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<_binder_ret_0>::value) {
				static pybind11::detail::override_caster_t<_binder_ret_0> caster;
				return pybind11::detail::cast_ref<_binder_ret_0>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<_binder_ret_0>(std::move(o));
		}
		return PolyMesh::points();
	}
};

// easy3d::PolyMesh::VertexProperty file:easy3d/core/poly_mesh.h line:206
struct PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_PolyMesh_VertexConnectivity_t : public easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity> {
	using easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::VertexProperty;

	struct easy3d::PolyMesh::VertexConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::VertexConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::VertexConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::PolyMesh::VertexConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::PolyMesh::VertexConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};

// easy3d::PolyMesh::VertexProperty file:easy3d/core/poly_mesh.h line:206
struct PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_Vec_3_float_t : public easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>> {
	using easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::VertexProperty;

	using _binder_ret_0 = easy3d::Vec<3, float> &;
	_binder_ret_0 operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>> *>(this), "__getitem__");
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

// easy3d::PolyMesh::EdgeProperty file:easy3d/core/poly_mesh.h line:230
struct PyCallBack_easy3d_PolyMesh_EdgeProperty_easy3d_PolyMesh_EdgeConnectivity_t : public easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity> {
	using easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::EdgeProperty;

	struct easy3d::PolyMesh::EdgeConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::EdgeConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::EdgeConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::PolyMesh::EdgeConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::PolyMesh::EdgeConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};

// easy3d::PolyMesh::HalfFaceProperty file:easy3d/core/poly_mesh.h line:254
struct PyCallBack_easy3d_PolyMesh_HalfFaceProperty_easy3d_PolyMesh_HalfFaceConnectivity_t : public easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity> {
	using easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::HalfFaceProperty;

	struct easy3d::PolyMesh::HalfFaceConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::HalfFaceConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::HalfFaceConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::PolyMesh::HalfFaceConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::PolyMesh::HalfFaceConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};

// easy3d::PolyMesh::CellProperty file:easy3d/core/poly_mesh.h line:302
struct PyCallBack_easy3d_PolyMesh_CellProperty_easy3d_PolyMesh_CellConnectivity_t : public easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity> {
	using easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::CellProperty;

	struct easy3d::PolyMesh::CellConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::CellConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::CellConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::PolyMesh::CellConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::PolyMesh::CellConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};



// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_PolyMesh_VertexConnectivity_t : public easy3d::Property<easy3d::PolyMesh::VertexConnectivity> {
    using easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::Property;

    struct easy3d::PolyMesh::VertexConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::PolyMesh::VertexConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::VertexConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::VertexConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::PolyMesh::VertexConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::PolyMesh::VertexConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_PolyMesh_EdgeConnectivity_t : public easy3d::Property<easy3d::PolyMesh::EdgeConnectivity> {
    using easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::Property;

    struct easy3d::PolyMesh::EdgeConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::PolyMesh::EdgeConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::PolyMesh::EdgeConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::PolyMesh::EdgeConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::PolyMesh::EdgeConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::PolyMesh::EdgeConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_HalfFaceConnectivity_t : public easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> {
    using easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> *>(this), "type");
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
struct PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_CellConnectivity_t : public easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> {
    using easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> *>(this), "type");
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
struct PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_VertexConnectivity_t : public easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> {
    using easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> *>(this), "type");
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
struct PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_EdgeConnectivity_t : public easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> {
    using easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> *>(this), "type");
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


void bind_easy3d_core_poly_mesh(pybind11::module_& m)
{
	{ // easy3d::PolyMesh file:easy3d/core/poly_mesh.h line:49
		pybind11::class_<easy3d::PolyMesh, std::shared_ptr<easy3d::PolyMesh>, PyCallBack_easy3d_PolyMesh, easy3d::Model> cl(m, "PolyMesh", "Data structure representing a polyhedral mesh.\n \n\n\n \n PolyMesh assumes the half-face normals pointing outside the cells.\n\n This implementation is inspired by Surface_mesh\n https://opensource.cit-ec.de/projects/surface_mesh");
		cl.def( pybind11::init( [](){ return new easy3d::PolyMesh(); }, [](){ return new PyCallBack_easy3d_PolyMesh(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_PolyMesh const &o){ return new PyCallBack_easy3d_PolyMesh(o); } ) );
		cl.def( pybind11::init( [](easy3d::PolyMesh const &o){ return new easy3d::PolyMesh(o); } ) );

        cl.def("name", [](easy3d::PolyMesh& self) { return self.name(); }, pybind11::return_value_policy::copy, "Get the name of the polyhedral mesh.");
        cl.def("set_name", [](easy3d::PolyMesh& self, const std::string& name) { self.set_name(name); }, "Set the name of the polyhedral mesh.");

        cl.def("renderer", [](const easy3d::PolyMesh& self) -> const easy3d::Renderer* { return self.renderer(); }, pybind11::return_value_policy::reference_internal, "Returns the renderer of the model");
        cl.def("renderer", [](easy3d::PolyMesh& self) -> easy3d::Renderer* { return self.renderer(); }, pybind11::return_value_policy::reference_internal, "Returns the renderer of the model");

        cl.def("assign", (class easy3d::PolyMesh & (easy3d::PolyMesh::*)(const class easy3d::PolyMesh &)) &easy3d::PolyMesh::operator=, "assign  to  performs a deep copy of all properties.\n\nC++: easy3d::PolyMesh::operator=(const class easy3d::PolyMesh &) --> class easy3d::PolyMesh &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("assign", (class easy3d::PolyMesh & (easy3d::PolyMesh::*)(const class easy3d::PolyMesh &)) &easy3d::PolyMesh::assign, "assign  to  does not copy custom properties.\n\nC++: easy3d::PolyMesh::assign(const class easy3d::PolyMesh &) --> class easy3d::PolyMesh &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("read", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::read, "Read mesh from a PM file \n Mainly for quick debug purposes. Client code should use PolyMeshIO.\n \n\n PolyMeshIO.\n\nC++: easy3d::PolyMesh::read(const std::string &) --> bool", pybind11::arg("filename"));
		cl.def("write", (bool (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::write, "Write mesh to a PM file \n Mainly for quick debug purposes. Client code should use PolyMeshIO.\n \n\n PolyMeshIO.\n\nC++: easy3d::PolyMesh::write(const std::string &) const --> bool", pybind11::arg("filename"));
		cl.def("add_vertex", (struct easy3d::PolyMesh::Vertex (easy3d::PolyMesh::*)(const class easy3d::Vec<3, float> &)) &easy3d::PolyMesh::add_vertex, "add a new vertex with position \n\nC++: easy3d::PolyMesh::add_vertex(const class easy3d::Vec<3, float> &) --> struct easy3d::PolyMesh::Vertex", pybind11::arg("p"));
		cl.def("add_cell", (struct easy3d::PolyMesh::Cell (easy3d::PolyMesh::*)(const class std::vector<struct easy3d::PolyMesh::HalfFace> &)) &easy3d::PolyMesh::add_cell, "add a new cell defined by \n \n\n The input faces created by add_face(), add_triangle(), or add_quad().\n \n\n add_face(), add_triangle(), add_quad(), add_tetra(), add_hexa()\n\nC++: easy3d::PolyMesh::add_cell(const class std::vector<struct easy3d::PolyMesh::HalfFace> &) --> struct easy3d::PolyMesh::Cell", pybind11::arg("faces"));
		cl.def("add_tetra", (struct easy3d::PolyMesh::Cell (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace, struct easy3d::PolyMesh::HalfFace, struct easy3d::PolyMesh::HalfFace, struct easy3d::PolyMesh::HalfFace)) &easy3d::PolyMesh::add_tetra, "add a new tetrahedron defined by its faces.\n \n\n f1, f2, f3} The input faces created by add_face() or add_triangle().\n \n\n add_cell(), add_tetra(), add_face(), add_triangle().\n\nC++: easy3d::PolyMesh::add_tetra(struct easy3d::PolyMesh::HalfFace, struct easy3d::PolyMesh::HalfFace, struct easy3d::PolyMesh::HalfFace, struct easy3d::PolyMesh::HalfFace) --> struct easy3d::PolyMesh::Cell", pybind11::arg("f0"), pybind11::arg("f1"), pybind11::arg("f2"), pybind11::arg("f3"));
		cl.def("add_tetra", (struct easy3d::PolyMesh::Cell (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex)) &easy3d::PolyMesh::add_tetra, "add a new tetrahedron connecting vertices    \n \n\n v1, v2, v3} The input vertices created by add_vertex().\n \n\n It creates all the faces and the cell, and adds them to the mesh.\n \n\n add_vertex()\n\nC++: easy3d::PolyMesh::add_tetra(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex) --> struct easy3d::PolyMesh::Cell", pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"));
		cl.def("add_hexa", (struct easy3d::PolyMesh::Cell (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex)) &easy3d::PolyMesh::add_hexa, "add a new hexahedron connecting vertices        \n \n\n v1, v2, v3, v4, v5, v6, v7} The input vertices created by add_vertex().\n     The vertices must be ordered as bellow:\n             3--------------------2\n             |\\                   |\\\n        ///             | \\                  | \\\n        ///             |  \\                 |  \\\n        ///             |   7----------------+---6\n             |   |                |   |\n             0---+----------------1   |\n              \\  |                 \\  |\n               \\ |                  \\ |\n                \\|                   \\|\n                 4--------------------5\n \n\n It creates all the faces and the cell, and adds them to the mesh.\n \n\n add_vertex()\n\nC++: easy3d::PolyMesh::add_hexa(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex) --> struct easy3d::PolyMesh::Cell", pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"), pybind11::arg("v4"), pybind11::arg("v5"), pybind11::arg("v6"), pybind11::arg("v7"));
		cl.def("add_face", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::*)(const class std::vector<struct easy3d::PolyMesh::Vertex> &)) &easy3d::PolyMesh::add_face, "add a new face connecting \n \n\n The input vertices created by add_vertex().\n \n\n add_triangle(), add_quad()\n\nC++: easy3d::PolyMesh::add_face(const class std::vector<struct easy3d::PolyMesh::Vertex> &) --> struct easy3d::PolyMesh::HalfFace", pybind11::arg("vertices"));
		cl.def("add_triangle", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex)) &easy3d::PolyMesh::add_triangle, "add a new triangle face connecting vertices   \n \n\n v1, v2} The input vertices created by add_vertex().\n \n\n add_face(), add_quad()\n\nC++: easy3d::PolyMesh::add_triangle(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex) --> struct easy3d::PolyMesh::HalfFace", pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"));
		cl.def("add_quad", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex)) &easy3d::PolyMesh::add_quad, "add a new quad face connecting vertices    \n \n\n v1, v2, v3} The input vertices created by add_vertex().\n \n\n add_face(), add_triangle()\n\nC++: easy3d::PolyMesh::add_quad(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex) --> struct easy3d::PolyMesh::HalfFace", pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"));
		cl.def("n_vertices", (unsigned int (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::n_vertices, "returns number of vertices in the mesh\n\nC++: easy3d::PolyMesh::n_vertices() const --> unsigned int");
		cl.def("n_edges", (unsigned int (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::n_edges, "returns number of edges in the mesh\n\nC++: easy3d::PolyMesh::n_edges() const --> unsigned int");
		cl.def("n_halffaces", (unsigned int (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::n_halffaces, "returns number of halffaces in the mesh\n\nC++: easy3d::PolyMesh::n_halffaces() const --> unsigned int");
		cl.def("n_faces", (unsigned int (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::n_faces, "returns number of faces in the mesh\n\nC++: easy3d::PolyMesh::n_faces() const --> unsigned int");
		cl.def("n_cells", (unsigned int (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::n_cells, "returns number of cells in the mesh\n\nC++: easy3d::PolyMesh::n_cells() const --> unsigned int");
		cl.def("clear", (void (easy3d::PolyMesh::*)()) &easy3d::PolyMesh::clear, "Removes all vertices, edges, halffaces, faces, cells and properties.\n \n\n After calling this method, the mesh is the same as newly constructed. The additional properties\n (such as normal vectors) are also removed and must thus be re-added if needed.\n\nC++: easy3d::PolyMesh::clear() --> void");
		cl.def("resize", (void (easy3d::PolyMesh::*)(unsigned int, unsigned int, unsigned int, unsigned int)) &easy3d::PolyMesh::resize, "resizes space for vertices, edges, halffaces, and their currently\n associated properties.\n Note: nf is the number of faces. for halffaces, nh = 2 * nf. */\n\nC++: easy3d::PolyMesh::resize(unsigned int, unsigned int, unsigned int, unsigned int) --> void", pybind11::arg("nv"), pybind11::arg("ne"), pybind11::arg("nf"), pybind11::arg("nc"));
		cl.def("is_valid", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::is_valid, "return whether vertex  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::PolyMesh::is_valid(struct easy3d::PolyMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_valid", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Edge) const) &easy3d::PolyMesh::is_valid, "return whether edge  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::PolyMesh::is_valid(struct easy3d::PolyMesh::Edge) const --> bool", pybind11::arg("e"));
		cl.def("is_valid", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::is_valid, "return whether halfface  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::PolyMesh::is_valid(struct easy3d::PolyMesh::HalfFace) const --> bool", pybind11::arg("h"));
		cl.def("is_valid", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Face) const) &easy3d::PolyMesh::is_valid, "return whether face  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::PolyMesh::is_valid(struct easy3d::PolyMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("is_valid", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Cell) const) &easy3d::PolyMesh::is_valid, "return whether cell  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::PolyMesh::is_valid(struct easy3d::PolyMesh::Cell) const --> bool", pybind11::arg("c"));
		cl.def("remove_vertex_property", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::remove_vertex_property, "remove the vertex property named \n\nC++: easy3d::PolyMesh::remove_vertex_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_edge_property", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::remove_edge_property, "remove the edge property named \n\nC++: easy3d::PolyMesh::remove_edge_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_halfface_property", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::remove_halfface_property, "remove the halfface property named \n\nC++: easy3d::PolyMesh::remove_halfface_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_face_property", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::remove_face_property, "remove the face property named \n\nC++: easy3d::PolyMesh::remove_face_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_cell_property", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::remove_cell_property, "remove the cell property named \n\nC++: easy3d::PolyMesh::remove_cell_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_model_property", (bool (easy3d::PolyMesh::*)(const std::string &)) &easy3d::PolyMesh::remove_model_property, "remove the model property named \n\nC++: easy3d::PolyMesh::remove_model_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("rename_vertex_property", (bool (easy3d::PolyMesh::*)(const std::string &, const std::string &)) &easy3d::PolyMesh::rename_vertex_property, "rename a vertex property given its name\n\nC++: easy3d::PolyMesh::rename_vertex_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_halfface_property", (bool (easy3d::PolyMesh::*)(const std::string &, const std::string &)) &easy3d::PolyMesh::rename_halfface_property, "rename a halfface property given its name\n\nC++: easy3d::PolyMesh::rename_halfface_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_face_property", (bool (easy3d::PolyMesh::*)(const std::string &, const std::string &)) &easy3d::PolyMesh::rename_face_property, "rename a face property given its name\n\nC++: easy3d::PolyMesh::rename_face_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_edge_property", (bool (easy3d::PolyMesh::*)(const std::string &, const std::string &)) &easy3d::PolyMesh::rename_edge_property, "rename an edge property given its name\n\nC++: easy3d::PolyMesh::rename_edge_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_cell_property", (bool (easy3d::PolyMesh::*)(const std::string &, const std::string &)) &easy3d::PolyMesh::rename_cell_property, "rename a cell property given its name\n\nC++: easy3d::PolyMesh::rename_cell_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_model_property", (bool (easy3d::PolyMesh::*)(const std::string &, const std::string &)) &easy3d::PolyMesh::rename_model_property, "rename a model property given its name\n\nC++: easy3d::PolyMesh::rename_model_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("get_vertex_property_type", (const class std::type_info & (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::get_vertex_property_type, "get the type_info  of vertex property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::PolyMesh::get_vertex_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_edge_property_type", (const class std::type_info & (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::get_edge_property_type, "get the type_info  of edge property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::PolyMesh::get_edge_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_halfface_property_type", (const class std::type_info & (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::get_halfface_property_type, "get the type_info  of halfface property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::PolyMesh::get_halfface_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_face_property_type", (const class std::type_info & (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::get_face_property_type, "get the type_info  of face property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::PolyMesh::get_face_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_cell_property_type", (const class std::type_info & (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::get_cell_property_type, "get the type_info  of cell property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::PolyMesh::get_cell_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_model_property_type", (const class std::type_info & (easy3d::PolyMesh::*)(const std::string &) const) &easy3d::PolyMesh::get_model_property_type, "get the type_info  of model property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::PolyMesh::get_model_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("vertex_properties", (class std::vector<std::string> (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::vertex_properties, "returns the names of all vertex properties\n\nC++: easy3d::PolyMesh::vertex_properties() const --> class std::vector<std::string>");
		cl.def("edge_properties", (class std::vector<std::string> (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::edge_properties, "returns the names of all edge properties\n\nC++: easy3d::PolyMesh::edge_properties() const --> class std::vector<std::string>");
		cl.def("halfface_properties", (class std::vector<std::string> (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::halfface_properties, "returns the names of all halfface properties\n\nC++: easy3d::PolyMesh::halfface_properties() const --> class std::vector<std::string>");
		cl.def("face_properties", (class std::vector<std::string> (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::face_properties, "returns the names of all face properties\n\nC++: easy3d::PolyMesh::face_properties() const --> class std::vector<std::string>");
		cl.def("cell_properties", (class std::vector<std::string> (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::cell_properties, "returns the names of all cell properties\n\nC++: easy3d::PolyMesh::cell_properties() const --> class std::vector<std::string>");
		cl.def("model_properties", (class std::vector<std::string> (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::model_properties, "returns the names of all model properties\n\nC++: easy3d::PolyMesh::model_properties() const --> class std::vector<std::string>");
		cl.def("property_stats", (void (easy3d::PolyMesh::*)(std::ostream &) const) &easy3d::PolyMesh::property_stats, "prints the names of all properties to an output stream (e.g., std::cout).\n\nC++: easy3d::PolyMesh::property_stats(std::ostream &) const --> void", pybind11::arg("output"));
		cl.def("vertices_begin", (class easy3d::PolyMesh::VertexIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::vertices_begin, "returns start iterator for vertices\n\nC++: easy3d::PolyMesh::vertices_begin() const --> class easy3d::PolyMesh::VertexIterator");
		cl.def("vertices_end", (class easy3d::PolyMesh::VertexIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::vertices_end, "returns end iterator for vertices\n\nC++: easy3d::PolyMesh::vertices_end() const --> class easy3d::PolyMesh::VertexIterator");
		cl.def("vertices", (class easy3d::PolyMesh::VertexContainer (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::vertices, "returns vertex container for C++11 range-based for-loops\n\nC++: easy3d::PolyMesh::vertices() const --> class easy3d::PolyMesh::VertexContainer");
		cl.def("edges_begin", (class easy3d::PolyMesh::EdgeIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::edges_begin, "returns start iterator for edges\n\nC++: easy3d::PolyMesh::edges_begin() const --> class easy3d::PolyMesh::EdgeIterator");
		cl.def("edges_end", (class easy3d::PolyMesh::EdgeIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::edges_end, "returns end iterator for edges\n\nC++: easy3d::PolyMesh::edges_end() const --> class easy3d::PolyMesh::EdgeIterator");
		cl.def("edges", (class easy3d::PolyMesh::EdgeContainer (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::edges, "returns edge container for C++11 range-based for-loops\n\nC++: easy3d::PolyMesh::edges() const --> class easy3d::PolyMesh::EdgeContainer");
		cl.def("halffaces_begin", (class easy3d::PolyMesh::HalfFaceIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::halffaces_begin, "returns start iterator for halffaces\n\nC++: easy3d::PolyMesh::halffaces_begin() const --> class easy3d::PolyMesh::HalfFaceIterator");
		cl.def("halffaces_end", (class easy3d::PolyMesh::HalfFaceIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::halffaces_end, "returns end iterator for halffaces\n\nC++: easy3d::PolyMesh::halffaces_end() const --> class easy3d::PolyMesh::HalfFaceIterator");
		cl.def("halffaces", (class easy3d::PolyMesh::HalffaceContainer (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::halffaces, "returns halfface container for C++11 range-based for-loops\n\nC++: easy3d::PolyMesh::halffaces() const --> class easy3d::PolyMesh::HalffaceContainer");
		cl.def("faces_begin", (class easy3d::PolyMesh::FaceIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::faces_begin, "returns start iterator for faces\n\nC++: easy3d::PolyMesh::faces_begin() const --> class easy3d::PolyMesh::FaceIterator");
		cl.def("faces_end", (class easy3d::PolyMesh::FaceIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::faces_end, "returns end iterator for faces\n\nC++: easy3d::PolyMesh::faces_end() const --> class easy3d::PolyMesh::FaceIterator");
		cl.def("faces", (class easy3d::PolyMesh::FaceContainer (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::faces, "returns face container for C++11 range-based for-loops\n\nC++: easy3d::PolyMesh::faces() const --> class easy3d::PolyMesh::FaceContainer");
		cl.def("cells_begin", (class easy3d::PolyMesh::CellIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::cells_begin, "returns start iterator for cells\n\nC++: easy3d::PolyMesh::cells_begin() const --> class easy3d::PolyMesh::CellIterator");
		cl.def("cells_end", (class easy3d::PolyMesh::CellIterator (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::cells_end, "returns end iterator for cells\n\nC++: easy3d::PolyMesh::cells_end() const --> class easy3d::PolyMesh::CellIterator");
		cl.def("cells", (class easy3d::PolyMesh::CellContainer (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::cells, "returns cell container for C++11 range-based for-loops\n\nC++: easy3d::PolyMesh::cells() const --> class easy3d::PolyMesh::CellContainer");
		cl.def("vertices", (const class std::set<struct easy3d::PolyMesh::Vertex> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::vertices, "returns the vertices around vertex \n\nC++: easy3d::PolyMesh::vertices(struct easy3d::PolyMesh::Vertex) const --> const class std::set<struct easy3d::PolyMesh::Vertex> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("halfface", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Face, unsigned int) const) &easy3d::PolyMesh::halfface, "returns the  halfface of face   has to be 0 or 1.\n\nC++: easy3d::PolyMesh::halfface(struct easy3d::PolyMesh::Face, unsigned int) const --> struct easy3d::PolyMesh::HalfFace", pybind11::arg("f"), pybind11::arg("i"));
		cl.def("face", (struct easy3d::PolyMesh::Face (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::face, "returns the face of HalfFace \n\nC++: easy3d::PolyMesh::face(struct easy3d::PolyMesh::HalfFace) const --> struct easy3d::PolyMesh::Face", pybind11::arg("h"));
		cl.def("opposite", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::opposite, "returns the twin halfface of halfface \n\nC++: easy3d::PolyMesh::opposite(struct easy3d::PolyMesh::HalfFace) const --> struct easy3d::PolyMesh::HalfFace", pybind11::arg("h"));
		cl.def("vertex", (struct easy3d::PolyMesh::Vertex (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Edge, unsigned int) const) &easy3d::PolyMesh::vertex, "returns the  vertex of edge   has to be 0 or 1.\n\nC++: easy3d::PolyMesh::vertex(struct easy3d::PolyMesh::Edge, unsigned int) const --> struct easy3d::PolyMesh::Vertex", pybind11::arg("e"), pybind11::arg("i"));
		cl.def("vertices", (const class std::vector<struct easy3d::PolyMesh::Vertex> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::vertices, "returns the set of vertices around halfface \n The vertices are ordered in a way such that its normal points outside of the cell associated with \n\nC++: easy3d::PolyMesh::vertices(struct easy3d::PolyMesh::HalfFace) const --> const class std::vector<struct easy3d::PolyMesh::Vertex> &", pybind11::return_value_policy::automatic, pybind11::arg("h"));
		cl.def("vertices", (const class std::vector<struct easy3d::PolyMesh::Vertex> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Face) const) &easy3d::PolyMesh::vertices, "returns the set of vertices around face \n\nC++: easy3d::PolyMesh::vertices(struct easy3d::PolyMesh::Face) const --> const class std::vector<struct easy3d::PolyMesh::Vertex> &", pybind11::return_value_policy::automatic, pybind11::arg("f"));
		cl.def("vertices", (const class std::set<struct easy3d::PolyMesh::Vertex> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Cell) const) &easy3d::PolyMesh::vertices, "returns the set of vertices around cell \n\nC++: easy3d::PolyMesh::vertices(struct easy3d::PolyMesh::Cell) const --> const class std::set<struct easy3d::PolyMesh::Vertex> &", pybind11::return_value_policy::automatic, pybind11::arg("c"));
		cl.def("edges", (const class std::set<struct easy3d::PolyMesh::Edge> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::edges, "returns the set of edges around vertex \n\nC++: easy3d::PolyMesh::edges(struct easy3d::PolyMesh::Vertex) const --> const class std::set<struct easy3d::PolyMesh::Edge> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("edges", (const class std::set<struct easy3d::PolyMesh::Edge> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::edges, "returns the set of edges around halfface \n\nC++: easy3d::PolyMesh::edges(struct easy3d::PolyMesh::HalfFace) const --> const class std::set<struct easy3d::PolyMesh::Edge> &", pybind11::return_value_policy::automatic, pybind11::arg("h"));
		cl.def("edges", (const class std::set<struct easy3d::PolyMesh::Edge> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Cell) const) &easy3d::PolyMesh::edges, "returns the set of edges around cell \n\nC++: easy3d::PolyMesh::edges(struct easy3d::PolyMesh::Cell) const --> const class std::set<struct easy3d::PolyMesh::Edge> &", pybind11::return_value_policy::automatic, pybind11::arg("c"));
		cl.def("halffaces", (const class std::set<struct easy3d::PolyMesh::HalfFace> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::halffaces, "returns the set of halffaces around vertex \n\nC++: easy3d::PolyMesh::halffaces(struct easy3d::PolyMesh::Vertex) const --> const class std::set<struct easy3d::PolyMesh::HalfFace> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("halffaces", (const class std::set<struct easy3d::PolyMesh::HalfFace> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Edge) const) &easy3d::PolyMesh::halffaces, "returns the set of halffaces around edge \n\nC++: easy3d::PolyMesh::halffaces(struct easy3d::PolyMesh::Edge) const --> const class std::set<struct easy3d::PolyMesh::HalfFace> &", pybind11::return_value_policy::automatic, pybind11::arg("e"));
		cl.def("halffaces", (const class std::vector<struct easy3d::PolyMesh::HalfFace> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Cell) const) &easy3d::PolyMesh::halffaces, "returns the set of halffaces around cell \n\nC++: easy3d::PolyMesh::halffaces(struct easy3d::PolyMesh::Cell) const --> const class std::vector<struct easy3d::PolyMesh::HalfFace> &", pybind11::return_value_policy::automatic, pybind11::arg("c"));
		cl.def("cells", (const class std::set<struct easy3d::PolyMesh::Cell> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::cells, "returns the set of cells around vertex \n\nC++: easy3d::PolyMesh::cells(struct easy3d::PolyMesh::Vertex) const --> const class std::set<struct easy3d::PolyMesh::Cell> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("cells", (const class std::set<struct easy3d::PolyMesh::Cell> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Edge) const) &easy3d::PolyMesh::cells, "returns the set of cells around edge \n\nC++: easy3d::PolyMesh::cells(struct easy3d::PolyMesh::Edge) const --> const class std::set<struct easy3d::PolyMesh::Cell> &", pybind11::return_value_policy::automatic, pybind11::arg("e"));
		cl.def("cell", (struct easy3d::PolyMesh::Cell (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::cell, "returns the cell associated with halfface \n\nC++: easy3d::PolyMesh::cell(struct easy3d::PolyMesh::HalfFace) const --> struct easy3d::PolyMesh::Cell", pybind11::arg("h"));
		cl.def("is_tetraheral_mesh", (bool (easy3d::PolyMesh::*)() const) &easy3d::PolyMesh::is_tetraheral_mesh, "returns whether the mesh a tetrahedral mesh, i.e., every cell is a tetrahedron.\n\nC++: easy3d::PolyMesh::is_tetraheral_mesh() const --> bool");
		cl.def("is_border", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::is_border, "returns whether  is a boundary vertex, i.e., at least one of its incident halfface\n is not associated with a cell.\n\nC++: easy3d::PolyMesh::is_border(struct easy3d::PolyMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_border", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Edge) const) &easy3d::PolyMesh::is_border, "returns whether  is a boundary edge, i.e., at least one of its incident halfface\n is not associated with a cell.\n\nC++: easy3d::PolyMesh::is_border(struct easy3d::PolyMesh::Edge) const --> bool", pybind11::arg("e"));
		cl.def("is_border", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::HalfFace) const) &easy3d::PolyMesh::is_border, "returns whether  is a boundary face, i.e., it is not associated with a cell.\n\nC++: easy3d::PolyMesh::is_border(struct easy3d::PolyMesh::HalfFace) const --> bool", pybind11::arg("h"));
		cl.def("is_border", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Face) const) &easy3d::PolyMesh::is_border, "returns whether  is a boundary face, i.e., it is incident to only one cell.\n\nC++: easy3d::PolyMesh::is_border(struct easy3d::PolyMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("find_edge", (struct easy3d::PolyMesh::Edge (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::find_edge, "find the edge (a,b)\n\nC++: easy3d::PolyMesh::find_edge(struct easy3d::PolyMesh::Vertex, struct easy3d::PolyMesh::Vertex) const --> struct easy3d::PolyMesh::Edge", pybind11::arg("a"), pybind11::arg("b"));
		cl.def("find_half_face", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::*)(const class std::vector<struct easy3d::PolyMesh::Vertex> &) const) &easy3d::PolyMesh::find_half_face, "find the halfface defined by a sequence of  (orientation sensitive)\n\nC++: easy3d::PolyMesh::find_half_face(const class std::vector<struct easy3d::PolyMesh::Vertex> &) const --> struct easy3d::PolyMesh::HalfFace", pybind11::arg("vertices"));
		cl.def("is_degenerate", (bool (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Face) const) &easy3d::PolyMesh::is_degenerate, "returns whether face  is degenerate\n\nC++: easy3d::PolyMesh::is_degenerate(struct easy3d::PolyMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("extract_boundary", (void (easy3d::PolyMesh::*)(class std::vector<class std::vector<struct easy3d::PolyMesh::Vertex> > &) const) &easy3d::PolyMesh::extract_boundary, "extracts the boundary surface and return its faces\n\nC++: easy3d::PolyMesh::extract_boundary(class std::vector<class std::vector<struct easy3d::PolyMesh::Vertex> > &) const --> void", pybind11::arg("faces"));
		cl.def("position", (const class easy3d::Vec<3, float> & (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Vertex) const) &easy3d::PolyMesh::position, "position of a vertex (read only)\n\nC++: easy3d::PolyMesh::position(struct easy3d::PolyMesh::Vertex) const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("points", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::PolyMesh::*)()) &easy3d::PolyMesh::points, "vector of vertex positions\n\nC++: easy3d::PolyMesh::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
		cl.def("update_face_normals", (void (easy3d::PolyMesh::*)()) &easy3d::PolyMesh::update_face_normals, "compute face normals by calling compute_face_normal(HalfFace) for each face.\n\nC++: easy3d::PolyMesh::update_face_normals() --> void");
		cl.def("compute_face_normal", (class easy3d::Vec<3, float> (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Face) const) &easy3d::PolyMesh::compute_face_normal, "compute normal vector of face \n\nC++: easy3d::PolyMesh::compute_face_normal(struct easy3d::PolyMesh::Face) const --> class easy3d::Vec<3, float>", pybind11::arg("f"));
		cl.def("update_vertex_normals", (void (easy3d::PolyMesh::*)()) &easy3d::PolyMesh::update_vertex_normals, "Computes vertex normals for each vertex.\n \n\n For vertices on the border of the polyhedral mesh, the normals point outside.\n      For interior vertices, vertex normals are not defined.\n      This method is not stable for concave vertices or vertices with spanning angles close to 0 or 180\n      degrees (but these are very rare cases for polyhedral meshes).\n\nC++: easy3d::PolyMesh::update_vertex_normals() --> void");
		cl.def("edge_length", (float (easy3d::PolyMesh::*)(struct easy3d::PolyMesh::Edge) const) &easy3d::PolyMesh::edge_length, "compute the length of edge \n\nC++: easy3d::PolyMesh::edge_length(struct easy3d::PolyMesh::Edge) const --> float", pybind11::arg("e"));

		{ // easy3d::PolyMesh::BaseHandle file:easy3d/core/poly_mesh.h line:57
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::BaseHandle, std::shared_ptr<easy3d::PolyMesh::BaseHandle>> cl(enclosing_class, "BaseHandle", "Base class for all topology types (internally it is basically an index)\n \n\n Vertex, Edge, HalfFace, Face, Cell");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::BaseHandle(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::BaseHandle const &o){ return new easy3d::PolyMesh::BaseHandle(o); } ) );
			cl.def("idx", (int (easy3d::PolyMesh::BaseHandle::*)() const) &easy3d::PolyMesh::BaseHandle::idx, "Get the underlying index of this handle\n\nC++: easy3d::PolyMesh::BaseHandle::idx() const --> int");
			cl.def("reset", (void (easy3d::PolyMesh::BaseHandle::*)()) &easy3d::PolyMesh::BaseHandle::reset, "reset handle to be invalid (index=-1)\n\nC++: easy3d::PolyMesh::BaseHandle::reset() --> void");
			cl.def("is_valid", (bool (easy3d::PolyMesh::BaseHandle::*)() const) &easy3d::PolyMesh::BaseHandle::is_valid, "return whether the handle is valid, i.e., the index is not equal to -1.\n\nC++: easy3d::PolyMesh::BaseHandle::is_valid() const --> bool");
			cl.def("__eq__", (bool (easy3d::PolyMesh::BaseHandle::*)(const class easy3d::PolyMesh::BaseHandle &) const) &easy3d::PolyMesh::BaseHandle::operator==, "are two handles equal?\n\nC++: easy3d::PolyMesh::BaseHandle::operator==(const class easy3d::PolyMesh::BaseHandle &) const --> bool", pybind11::arg("_rhs"));
			cl.def("__ne__", (bool (easy3d::PolyMesh::BaseHandle::*)(const class easy3d::PolyMesh::BaseHandle &) const) &easy3d::PolyMesh::BaseHandle::operator!=, "are two handles different?\n\nC++: easy3d::PolyMesh::BaseHandle::operator!=(const class easy3d::PolyMesh::BaseHandle &) const --> bool", pybind11::arg("_rhs"));
			cl.def("assign", (class easy3d::PolyMesh::BaseHandle & (easy3d::PolyMesh::BaseHandle::*)(const class easy3d::PolyMesh::BaseHandle &)) &easy3d::PolyMesh::BaseHandle::operator=, "C++: easy3d::PolyMesh::BaseHandle::operator=(const class easy3d::PolyMesh::BaseHandle &) --> class easy3d::PolyMesh::BaseHandle &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::PolyMesh::BaseHandle::Hash file:easy3d/core/poly_mesh.h line:89
            auto & enclosing_class = cl;
            pybind11::class_<easy3d::PolyMesh::BaseHandle::Hash, std::shared_ptr<easy3d::PolyMesh::BaseHandle::Hash>> cl(enclosing_class, "Hash", "helper structure to be able to use std::unordered_map");
            cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::BaseHandle::Hash(); } ) );
            cl.def("__call__", (std::size_t (easy3d::PolyMesh::BaseHandle::Hash::*)(const class easy3d::PolyMesh::BaseHandle &) const) &easy3d::PolyMesh::BaseHandle::Hash::operator(), "C++: easy3d::PolyMesh::BaseHandle::Hash::operator()(const class easy3d::PolyMesh::BaseHandle &) const --> std::size_t", pybind11::arg("h"));
        }

		{ // easy3d::PolyMesh::Vertex file:easy3d/core/poly_mesh.h line:101
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::Vertex, std::shared_ptr<easy3d::PolyMesh::Vertex>, easy3d::PolyMesh::BaseHandle> cl(enclosing_class, "Vertex", "this type represents a vertex (internally it is basically an index)\n  \n\n Edge, HalfFace, Face, Cell");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::Vertex(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::Vertex const &o){ return new easy3d::PolyMesh::Vertex(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::PolyMesh::Vertex::*)(std::ostream &) const) &easy3d::PolyMesh::Vertex::operator<<, "C++: easy3d::PolyMesh::Vertex::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::PolyMesh::Vertex & (easy3d::PolyMesh::Vertex::*)(const struct easy3d::PolyMesh::Vertex &)) &easy3d::PolyMesh::Vertex::operator=, "C++: easy3d::PolyMesh::Vertex::operator=(const struct easy3d::PolyMesh::Vertex &) --> struct easy3d::PolyMesh::Vertex &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::Edge file:easy3d/core/poly_mesh.h line:110
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::Edge, std::shared_ptr<easy3d::PolyMesh::Edge>, easy3d::PolyMesh::BaseHandle> cl(enclosing_class, "Edge", "this type represents an edge (internally it is basically an index)\n \n\n Vertex, HalfFace, Face, Cell");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::Edge(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::Edge const &o){ return new easy3d::PolyMesh::Edge(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::PolyMesh::Edge::*)(std::ostream &) const) &easy3d::PolyMesh::Edge::operator<<, "C++: easy3d::PolyMesh::Edge::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::PolyMesh::Edge & (easy3d::PolyMesh::Edge::*)(const struct easy3d::PolyMesh::Edge &)) &easy3d::PolyMesh::Edge::operator=, "C++: easy3d::PolyMesh::Edge::operator=(const struct easy3d::PolyMesh::Edge &) --> struct easy3d::PolyMesh::Edge &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::HalfFace file:easy3d/core/poly_mesh.h line:120
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::HalfFace, std::shared_ptr<easy3d::PolyMesh::HalfFace>, easy3d::PolyMesh::BaseHandle> cl(enclosing_class, "HalfFace", "this type represents a halfface (internally it is basically an index)\n \n\n Vertex, Edge, Face, Cell");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::HalfFace(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::HalfFace const &o){ return new easy3d::PolyMesh::HalfFace(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::PolyMesh::HalfFace::*)(std::ostream &) const) &easy3d::PolyMesh::HalfFace::operator<<, "C++: easy3d::PolyMesh::HalfFace::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::PolyMesh::HalfFace & (easy3d::PolyMesh::HalfFace::*)(const struct easy3d::PolyMesh::HalfFace &)) &easy3d::PolyMesh::HalfFace::operator=, "C++: easy3d::PolyMesh::HalfFace::operator=(const struct easy3d::PolyMesh::HalfFace &) --> struct easy3d::PolyMesh::HalfFace &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::Face file:easy3d/core/poly_mesh.h line:129
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::Face, std::shared_ptr<easy3d::PolyMesh::Face>, easy3d::PolyMesh::BaseHandle> cl(enclosing_class, "Face", "this type represents a face (internally it is basically an index)\n \n\n Vertex, Edge, HalfFace, Cell");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::Face(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::Face const &o){ return new easy3d::PolyMesh::Face(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::PolyMesh::Face::*)(std::ostream &) const) &easy3d::PolyMesh::Face::operator<<, "C++: easy3d::PolyMesh::Face::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
		}

		{ // easy3d::PolyMesh::Cell file:easy3d/core/poly_mesh.h line:138
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::Cell, std::shared_ptr<easy3d::PolyMesh::Cell>, easy3d::PolyMesh::BaseHandle> cl(enclosing_class, "Cell", "this type represents a polyhedral cell (internally it is basically an index)\n \n\n Vertex, Edge, HalfFace, Face");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::Cell(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::Cell const &o){ return new easy3d::PolyMesh::Cell(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::PolyMesh::Cell::*)(std::ostream &) const) &easy3d::PolyMesh::Cell::operator<<, "C++: easy3d::PolyMesh::Cell::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::PolyMesh::Cell & (easy3d::PolyMesh::Cell::*)(const struct easy3d::PolyMesh::Cell &)) &easy3d::PolyMesh::Cell::operator=, "C++: easy3d::PolyMesh::Cell::operator=(const struct easy3d::PolyMesh::Cell &) --> struct easy3d::PolyMesh::Cell &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::VertexConnectivity file:easy3d/core/poly_mesh.h line:151
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::VertexConnectivity, std::shared_ptr<easy3d::PolyMesh::VertexConnectivity>> cl(enclosing_class, "VertexConnectivity", "This type stores the vertex connectivity\n \n\n EdgeConnectivity, HalfFaceConnectivity, CellConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::VertexConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::VertexConnectivity const &o){ return new easy3d::PolyMesh::VertexConnectivity(o); } ) );
			cl.def_readwrite("vertices_", &easy3d::PolyMesh::VertexConnectivity::vertices_);
			cl.def_readwrite("edges_", &easy3d::PolyMesh::VertexConnectivity::edges_);
			cl.def_readwrite("halffaces_", &easy3d::PolyMesh::VertexConnectivity::halffaces_);
			cl.def_readwrite("cells_", &easy3d::PolyMesh::VertexConnectivity::cells_);
			cl.def("read", (void (easy3d::PolyMesh::VertexConnectivity::*)(std::istream &)) &easy3d::PolyMesh::VertexConnectivity::read, "C++: easy3d::PolyMesh::VertexConnectivity::read(std::istream &) --> void", pybind11::arg("in"));
			cl.def("write", (void (easy3d::PolyMesh::VertexConnectivity::*)(std::ostream &) const) &easy3d::PolyMesh::VertexConnectivity::write, "C++: easy3d::PolyMesh::VertexConnectivity::write(std::ostream &) const --> void", pybind11::arg("out"));
			cl.def("assign", (struct easy3d::PolyMesh::VertexConnectivity & (easy3d::PolyMesh::VertexConnectivity::*)(const struct easy3d::PolyMesh::VertexConnectivity &)) &easy3d::PolyMesh::VertexConnectivity::operator=, "C++: easy3d::PolyMesh::VertexConnectivity::operator=(const struct easy3d::PolyMesh::VertexConnectivity &) --> struct easy3d::PolyMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>, std::shared_ptr<easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>>, PyCallBack_easy3d_Property_easy3d_PolyMesh_EdgeConnectivity_t> cl(m, "Property_easy3d_PolyMesh_EdgeConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_EdgeConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_PolyMesh_EdgeConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_EdgeConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::PolyMesh::EdgeConnectivity> const &o){ return new easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::PolyMesh::EdgeConnectivity & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::PolyMesh::EdgeConnectivity * (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::data() const --> const struct easy3d::PolyMesh::EdgeConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::PolyMesh::EdgeConnectivity file:easy3d/core/poly_mesh.h line:165
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::EdgeConnectivity, std::shared_ptr<easy3d::PolyMesh::EdgeConnectivity>> cl(enclosing_class, "EdgeConnectivity", "This type stores the edge connectivity\n \n\n VertexConnectivity, HalfFaceConnectivity, CellConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::EdgeConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::EdgeConnectivity const &o){ return new easy3d::PolyMesh::EdgeConnectivity(o); } ) );
			cl.def_readwrite("vertices_", &easy3d::PolyMesh::EdgeConnectivity::vertices_);
			cl.def_readwrite("halffaces_", &easy3d::PolyMesh::EdgeConnectivity::halffaces_);
			cl.def_readwrite("cells_", &easy3d::PolyMesh::EdgeConnectivity::cells_);
			cl.def("read", (void (easy3d::PolyMesh::EdgeConnectivity::*)(std::istream &)) &easy3d::PolyMesh::EdgeConnectivity::read, "C++: easy3d::PolyMesh::EdgeConnectivity::read(std::istream &) --> void", pybind11::arg("in"));
			cl.def("write", (void (easy3d::PolyMesh::EdgeConnectivity::*)(std::ostream &) const) &easy3d::PolyMesh::EdgeConnectivity::write, "C++: easy3d::PolyMesh::EdgeConnectivity::write(std::ostream &) const --> void", pybind11::arg("out"));
			cl.def("assign", (struct easy3d::PolyMesh::EdgeConnectivity & (easy3d::PolyMesh::EdgeConnectivity::*)(const struct easy3d::PolyMesh::EdgeConnectivity &)) &easy3d::PolyMesh::EdgeConnectivity::operator=, "C++: easy3d::PolyMesh::EdgeConnectivity::operator=(const struct easy3d::PolyMesh::EdgeConnectivity &) --> struct easy3d::PolyMesh::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::HalfFaceConnectivity file:easy3d/core/poly_mesh.h line:177
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::HalfFaceConnectivity, std::shared_ptr<easy3d::PolyMesh::HalfFaceConnectivity>> cl(enclosing_class, "HalfFaceConnectivity", "This type stores the halfface connectivity\n \n\n VertexConnectivity, EdgeConnectivity, CellConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::HalfFaceConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::HalfFaceConnectivity const &o){ return new easy3d::PolyMesh::HalfFaceConnectivity(o); } ) );
			cl.def_readwrite("vertices_", &easy3d::PolyMesh::HalfFaceConnectivity::vertices_);
			cl.def_readwrite("edges_", &easy3d::PolyMesh::HalfFaceConnectivity::edges_);
			cl.def_readwrite("cell_", &easy3d::PolyMesh::HalfFaceConnectivity::cell_);
			cl.def_readwrite("opposite_", &easy3d::PolyMesh::HalfFaceConnectivity::opposite_);
			cl.def("read", (void (easy3d::PolyMesh::HalfFaceConnectivity::*)(std::istream &)) &easy3d::PolyMesh::HalfFaceConnectivity::read, "C++: easy3d::PolyMesh::HalfFaceConnectivity::read(std::istream &) --> void", pybind11::arg("in"));
			cl.def("write", (void (easy3d::PolyMesh::HalfFaceConnectivity::*)(std::ostream &) const) &easy3d::PolyMesh::HalfFaceConnectivity::write, "C++: easy3d::PolyMesh::HalfFaceConnectivity::write(std::ostream &) const --> void", pybind11::arg("out"));
			cl.def("assign", (struct easy3d::PolyMesh::HalfFaceConnectivity & (easy3d::PolyMesh::HalfFaceConnectivity::*)(const struct easy3d::PolyMesh::HalfFaceConnectivity &)) &easy3d::PolyMesh::HalfFaceConnectivity::operator=, "C++: easy3d::PolyMesh::HalfFaceConnectivity::operator=(const struct easy3d::PolyMesh::HalfFaceConnectivity &) --> struct easy3d::PolyMesh::HalfFaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::CellConnectivity file:easy3d/core/poly_mesh.h line:190
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::CellConnectivity, std::shared_ptr<easy3d::PolyMesh::CellConnectivity>> cl(enclosing_class, "CellConnectivity", "This type stores the cell connectivity\n \n\n VertexConnectivity, EdgeConnectivity, HalfFaceConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::CellConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::CellConnectivity const &o){ return new easy3d::PolyMesh::CellConnectivity(o); } ) );
			cl.def_readwrite("vertices_", &easy3d::PolyMesh::CellConnectivity::vertices_);
			cl.def_readwrite("edges_", &easy3d::PolyMesh::CellConnectivity::edges_);
			cl.def_readwrite("halffaces_", &easy3d::PolyMesh::CellConnectivity::halffaces_);
			cl.def("read", (void (easy3d::PolyMesh::CellConnectivity::*)(std::istream &)) &easy3d::PolyMesh::CellConnectivity::read, "C++: easy3d::PolyMesh::CellConnectivity::read(std::istream &) --> void", pybind11::arg("in"));
			cl.def("write", (void (easy3d::PolyMesh::CellConnectivity::*)(std::ostream &) const) &easy3d::PolyMesh::CellConnectivity::write, "C++: easy3d::PolyMesh::CellConnectivity::write(std::ostream &) const --> void", pybind11::arg("out"));
			cl.def("assign", (struct easy3d::PolyMesh::CellConnectivity & (easy3d::PolyMesh::CellConnectivity::*)(const struct easy3d::PolyMesh::CellConnectivity &)) &easy3d::PolyMesh::CellConnectivity::operator=, "C++: easy3d::PolyMesh::CellConnectivity::operator=(const struct easy3d::PolyMesh::CellConnectivity &) --> struct easy3d::PolyMesh::CellConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::PolyMesh::VertexConnectivity>, std::shared_ptr<easy3d::Property<easy3d::PolyMesh::VertexConnectivity>>, PyCallBack_easy3d_Property_easy3d_PolyMesh_VertexConnectivity_t> cl(m, "Property_easy3d_PolyMesh_VertexConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::PolyMesh::VertexConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_VertexConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_PolyMesh_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_VertexConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::PolyMesh::VertexConnectivity> const &o){ return new easy3d::Property<easy3d::PolyMesh::VertexConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::PolyMesh::VertexConnectivity & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::PolyMesh::VertexConnectivity * (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::data() const --> const struct easy3d::PolyMesh::VertexConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::PolyMesh::VertexProperty file:easy3d/core/poly_mesh.h line:206
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>, std::shared_ptr<easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>>, PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_PolyMesh_VertexConnectivity_t, easy3d::Property<easy3d::PolyMesh::VertexConnectivity>> cl(enclosing_class, "VertexProperty_easy3d_PolyMesh_VertexConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>(); }, [](){ return new PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_PolyMesh_VertexConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_PolyMesh_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_PolyMesh_VertexConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity> const &o){ return new easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::PolyMesh::VertexConnectivity & (easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::*)(struct easy3d::PolyMesh::Vertex)) &easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::operator[], "C++: easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::operator[](struct easy3d::PolyMesh::Vertex) --> struct easy3d::PolyMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::PolyMesh::VertexProperty<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::*)(const class easy3d::PolyMesh::VertexProperty<struct easy3d::PolyMesh::VertexConnectivity> &)) &easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::operator=, "C++: easy3d::PolyMesh::VertexProperty<easy3d::PolyMesh::VertexConnectivity>::operator=(const class easy3d::PolyMesh::VertexProperty<struct easy3d::PolyMesh::VertexConnectivity> &) --> class easy3d::PolyMesh::VertexProperty<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::PolyMesh::VertexConnectivity & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::PolyMesh::VertexConnectivity * (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::data() const --> const struct easy3d::PolyMesh::VertexConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::VertexConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::VertexProperty file:easy3d/core/poly_mesh.h line:206
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>, std::shared_ptr<easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>>, PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_Vec_3_float_t, easy3d::Property<easy3d::Vec<3, float>>> cl(enclosing_class, "VertexProperty_easy3d_Vec_3_float_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>(); }, [](){ return new PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_Vec_3_float_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<class easy3d::Vec<3, float> >>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_Vec_3_float_t const &o){ return new PyCallBack_easy3d_PolyMesh_VertexProperty_easy3d_Vec_3_float_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>> const &o){ return new easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>(o); } ) );
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::*)(struct easy3d::PolyMesh::Vertex)) &easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::operator[], "C++: easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::operator[](struct easy3d::PolyMesh::Vertex) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::PolyMesh::VertexProperty<class easy3d::Vec<3, float> > & (easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::*)(const class easy3d::PolyMesh::VertexProperty<class easy3d::Vec<3, float> > &)) &easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::operator=, "C++: easy3d::PolyMesh::VertexProperty<easy3d::Vec<3, float>>::operator=(const class easy3d::PolyMesh::VertexProperty<class easy3d::Vec<3, float> > &) --> class easy3d::PolyMesh::VertexProperty<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::reset, "C++: easy3d::Property<easy3d::Vec<3, float>>::reset() --> void");
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::Property<easy3d::Vec<3, float>>::*)(size_t)) &easy3d::Property<easy3d::Vec<3, float>>::operator[], "C++: easy3d::Property<easy3d::Vec<3, float>>::operator[](size_t) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const class easy3d::Vec<3, float> * (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::data, "C++: easy3d::Property<easy3d::Vec<3, float>>::data() const --> const class easy3d::Vec<3, float> *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::vector, "C++: easy3d::Property<easy3d::Vec<3, float>>::vector() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::array, "C++: easy3d::Property<easy3d::Vec<3, float>>::array() --> class easy3d::PropertyArray<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::name, "C++: easy3d::Property<easy3d::Vec<3, float>>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::Vec<3, float>>::*)(const std::string &)) &easy3d::Property<easy3d::Vec<3, float>>::set_name, "C++: easy3d::Property<easy3d::Vec<3, float>>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)(const class easy3d::Property<class easy3d::Vec<3, float> > &)) &easy3d::Property<easy3d::Vec<3, float>>::operator=, "C++: easy3d::Property<easy3d::Vec<3, float>>::operator=(const class easy3d::Property<class easy3d::Vec<3, float> > &) --> class easy3d::Property<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::EdgeProperty file:easy3d/core/poly_mesh.h line:230
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>, std::shared_ptr<easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>>, PyCallBack_easy3d_PolyMesh_EdgeProperty_easy3d_PolyMesh_EdgeConnectivity_t, easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>> cl(enclosing_class, "EdgeProperty_easy3d_PolyMesh_EdgeConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>(); }, [](){ return new PyCallBack_easy3d_PolyMesh_EdgeProperty_easy3d_PolyMesh_EdgeConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_PolyMesh_EdgeProperty_easy3d_PolyMesh_EdgeConnectivity_t const &o){ return new PyCallBack_easy3d_PolyMesh_EdgeProperty_easy3d_PolyMesh_EdgeConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity> const &o){ return new easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::PolyMesh::EdgeConnectivity & (easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::*)(struct easy3d::PolyMesh::Edge)) &easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::operator[], "C++: easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::operator[](struct easy3d::PolyMesh::Edge) --> struct easy3d::PolyMesh::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("e"));
			cl.def("assign", (class easy3d::PolyMesh::EdgeProperty<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::*)(const class easy3d::PolyMesh::EdgeProperty<struct easy3d::PolyMesh::EdgeConnectivity> &)) &easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::operator=, "C++: easy3d::PolyMesh::EdgeProperty<easy3d::PolyMesh::EdgeConnectivity>::operator=(const class easy3d::PolyMesh::EdgeProperty<struct easy3d::PolyMesh::EdgeConnectivity> &) --> class easy3d::PolyMesh::EdgeProperty<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::PolyMesh::EdgeConnectivity & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::PolyMesh::EdgeConnectivity * (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::data() const --> const struct easy3d::PolyMesh::EdgeConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::EdgeConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>, std::shared_ptr<easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>>, PyCallBack_easy3d_Property_easy3d_PolyMesh_HalfFaceConnectivity_t> cl(m, "Property_easy3d_PolyMesh_HalfFaceConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_HalfFaceConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_PolyMesh_HalfFaceConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_HalfFaceConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity> const &o){ return new easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::PolyMesh::HalfFaceConnectivity & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::HalfFaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::PolyMesh::HalfFaceConnectivity * (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::data() const --> const struct easy3d::PolyMesh::HalfFaceConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::PolyMesh::HalfFaceProperty file:easy3d/core/poly_mesh.h line:254
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>, std::shared_ptr<easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>>, PyCallBack_easy3d_PolyMesh_HalfFaceProperty_easy3d_PolyMesh_HalfFaceConnectivity_t, easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>> cl(enclosing_class, "HalfFaceProperty_easy3d_PolyMesh_HalfFaceConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>(); }, [](){ return new PyCallBack_easy3d_PolyMesh_HalfFaceProperty_easy3d_PolyMesh_HalfFaceConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_PolyMesh_HalfFaceProperty_easy3d_PolyMesh_HalfFaceConnectivity_t const &o){ return new PyCallBack_easy3d_PolyMesh_HalfFaceProperty_easy3d_PolyMesh_HalfFaceConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity> const &o){ return new easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::PolyMesh::HalfFaceConnectivity & (easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::*)(struct easy3d::PolyMesh::HalfFace)) &easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::operator[], "C++: easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::operator[](struct easy3d::PolyMesh::HalfFace) --> struct easy3d::PolyMesh::HalfFaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("h"));
			cl.def("assign", (class easy3d::PolyMesh::HalfFaceProperty<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const class easy3d::PolyMesh::HalfFaceProperty<struct easy3d::PolyMesh::HalfFaceConnectivity> &)) &easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::operator=, "C++: easy3d::PolyMesh::HalfFaceProperty<easy3d::PolyMesh::HalfFaceConnectivity>::operator=(const class easy3d::PolyMesh::HalfFaceProperty<struct easy3d::PolyMesh::HalfFaceConnectivity> &) --> class easy3d::PolyMesh::HalfFaceProperty<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::PolyMesh::HalfFaceConnectivity & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::HalfFaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::PolyMesh::HalfFaceConnectivity * (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::data() const --> const struct easy3d::PolyMesh::HalfFaceConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::HalfFaceConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::PolyMesh::CellConnectivity>, std::shared_ptr<easy3d::Property<easy3d::PolyMesh::CellConnectivity>>, PyCallBack_easy3d_Property_easy3d_PolyMesh_CellConnectivity_t> cl(m, "Property_easy3d_PolyMesh_CellConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::PolyMesh::CellConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_CellConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_PolyMesh_CellConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_PolyMesh_CellConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::PolyMesh::CellConnectivity> const &o){ return new easy3d::Property<easy3d::PolyMesh::CellConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::PolyMesh::CellConnectivity & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::CellConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::PolyMesh::CellConnectivity * (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::data() const --> const struct easy3d::PolyMesh::CellConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::PolyMesh::CellProperty file:easy3d/core/poly_mesh.h line:302
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>, std::shared_ptr<easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>>, PyCallBack_easy3d_PolyMesh_CellProperty_easy3d_PolyMesh_CellConnectivity_t, easy3d::Property<easy3d::PolyMesh::CellConnectivity>> cl(enclosing_class, "CellProperty_easy3d_PolyMesh_CellConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>(); }, [](){ return new PyCallBack_easy3d_PolyMesh_CellProperty_easy3d_PolyMesh_CellConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_PolyMesh_CellProperty_easy3d_PolyMesh_CellConnectivity_t const &o){ return new PyCallBack_easy3d_PolyMesh_CellProperty_easy3d_PolyMesh_CellConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity> const &o){ return new easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::PolyMesh::CellConnectivity & (easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::*)(struct easy3d::PolyMesh::Cell)) &easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::operator[], "C++: easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::operator[](struct easy3d::PolyMesh::Cell) --> struct easy3d::PolyMesh::CellConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("c"));
			cl.def("assign", (class easy3d::PolyMesh::CellProperty<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::*)(const class easy3d::PolyMesh::CellProperty<struct easy3d::PolyMesh::CellConnectivity> &)) &easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::operator=, "C++: easy3d::PolyMesh::CellProperty<easy3d::PolyMesh::CellConnectivity>::operator=(const class easy3d::PolyMesh::CellProperty<struct easy3d::PolyMesh::CellConnectivity> &) --> class easy3d::PolyMesh::CellProperty<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::reset, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::PolyMesh::CellConnectivity & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)(size_t)) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator[], "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::CellConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::PolyMesh::CellConnectivity * (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::data, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::data() const --> const struct easy3d::PolyMesh::CellConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::vector, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::array, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::name, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::set_name, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::Property<easy3d::PolyMesh::CellConnectivity>::*)(const class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> &)) &easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator=, "C++: easy3d::Property<easy3d::PolyMesh::CellConnectivity>::operator=(const class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> &) --> class easy3d::Property<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PolyMesh::VertexIterator file:easy3d/core/poly_mesh.h line:354
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::VertexIterator, std::shared_ptr<easy3d::PolyMesh::VertexIterator>> cl(enclosing_class, "VertexIterator", "this class iterates linearly over all vertices\n \n\n vertices_begin(), vertices_end()\n \n\n EdgeIterator, HalfFaceIterator, FaceIterator, CellIterator");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::VertexIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::PolyMesh::Vertex const & a0){ return new easy3d::PolyMesh::VertexIterator(a0); } ), "doc" , pybind11::arg("v"));
			cl.def( pybind11::init<struct easy3d::PolyMesh::Vertex, const class easy3d::PolyMesh *>(), pybind11::arg("v"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::VertexIterator const &o){ return new easy3d::PolyMesh::VertexIterator(o); } ) );
			cl.def("dereference", (struct easy3d::PolyMesh::Vertex (easy3d::PolyMesh::VertexIterator::*)() const) &easy3d::PolyMesh::VertexIterator::operator*, "get the vertex the iterator refers to\n\nC++: easy3d::PolyMesh::VertexIterator::operator*() const --> struct easy3d::PolyMesh::Vertex");
			cl.def("__eq__", (bool (easy3d::PolyMesh::VertexIterator::*)(const class easy3d::PolyMesh::VertexIterator &) const) &easy3d::PolyMesh::VertexIterator::operator==, "are two iterators equal?\n\nC++: easy3d::PolyMesh::VertexIterator::operator==(const class easy3d::PolyMesh::VertexIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::PolyMesh::VertexIterator::*)(const class easy3d::PolyMesh::VertexIterator &) const) &easy3d::PolyMesh::VertexIterator::operator!=, "are two iterators different?\n\nC++: easy3d::PolyMesh::VertexIterator::operator!=(const class easy3d::PolyMesh::VertexIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::PolyMesh::VertexIterator & (easy3d::PolyMesh::VertexIterator::*)()) &easy3d::PolyMesh::VertexIterator::operator++, "pre-increment iterator\n\nC++: easy3d::PolyMesh::VertexIterator::operator++() --> class easy3d::PolyMesh::VertexIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::PolyMesh::VertexIterator & (easy3d::PolyMesh::VertexIterator::*)()) &easy3d::PolyMesh::VertexIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::PolyMesh::VertexIterator::operator--() --> class easy3d::PolyMesh::VertexIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::PolyMesh::EdgeIterator file:easy3d/core/poly_mesh.h line:403
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::EdgeIterator, std::shared_ptr<easy3d::PolyMesh::EdgeIterator>> cl(enclosing_class, "EdgeIterator", "this class iterates linearly over all edges\n \n\n edges_begin(), edges_end()\n \n\n VertexIterator, HalfFaceIterator, FaceIterator, CellIterator");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::EdgeIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::PolyMesh::Edge const & a0){ return new easy3d::PolyMesh::EdgeIterator(a0); } ), "doc" , pybind11::arg("e"));
			cl.def( pybind11::init<struct easy3d::PolyMesh::Edge, const class easy3d::PolyMesh *>(), pybind11::arg("e"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::EdgeIterator const &o){ return new easy3d::PolyMesh::EdgeIterator(o); } ) );
			cl.def("dereference", (struct easy3d::PolyMesh::Edge (easy3d::PolyMesh::EdgeIterator::*)() const) &easy3d::PolyMesh::EdgeIterator::operator*, "get the edge the iterator refers to\n\nC++: easy3d::PolyMesh::EdgeIterator::operator*() const --> struct easy3d::PolyMesh::Edge");
			cl.def("__eq__", (bool (easy3d::PolyMesh::EdgeIterator::*)(const class easy3d::PolyMesh::EdgeIterator &) const) &easy3d::PolyMesh::EdgeIterator::operator==, "are two iterators equal?\n\nC++: easy3d::PolyMesh::EdgeIterator::operator==(const class easy3d::PolyMesh::EdgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::PolyMesh::EdgeIterator::*)(const class easy3d::PolyMesh::EdgeIterator &) const) &easy3d::PolyMesh::EdgeIterator::operator!=, "are two iterators different?\n\nC++: easy3d::PolyMesh::EdgeIterator::operator!=(const class easy3d::PolyMesh::EdgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::PolyMesh::EdgeIterator & (easy3d::PolyMesh::EdgeIterator::*)()) &easy3d::PolyMesh::EdgeIterator::operator++, "pre-increment iterator\n\nC++: easy3d::PolyMesh::EdgeIterator::operator++() --> class easy3d::PolyMesh::EdgeIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::PolyMesh::EdgeIterator & (easy3d::PolyMesh::EdgeIterator::*)()) &easy3d::PolyMesh::EdgeIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::PolyMesh::EdgeIterator::operator--() --> class easy3d::PolyMesh::EdgeIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::PolyMesh::HalfFaceIterator file:easy3d/core/poly_mesh.h line:452
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::HalfFaceIterator, std::shared_ptr<easy3d::PolyMesh::HalfFaceIterator>> cl(enclosing_class, "HalfFaceIterator", "this class iterates linearly over all halffaces\n \n\n halffaces_begin(), halffaces_end()\n \n\n VertexIterator, EdgeIterator, FaceIterator, CellIterator");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::HalfFaceIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::PolyMesh::HalfFace const & a0){ return new easy3d::PolyMesh::HalfFaceIterator(a0); } ), "doc" , pybind11::arg("h"));
			cl.def( pybind11::init<struct easy3d::PolyMesh::HalfFace, const class easy3d::PolyMesh *>(), pybind11::arg("h"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::HalfFaceIterator const &o){ return new easy3d::PolyMesh::HalfFaceIterator(o); } ) );
			cl.def("dereference", (struct easy3d::PolyMesh::HalfFace (easy3d::PolyMesh::HalfFaceIterator::*)() const) &easy3d::PolyMesh::HalfFaceIterator::operator*, "get the halfface the iterator refers to\n\nC++: easy3d::PolyMesh::HalfFaceIterator::operator*() const --> struct easy3d::PolyMesh::HalfFace");
			cl.def("__eq__", (bool (easy3d::PolyMesh::HalfFaceIterator::*)(const class easy3d::PolyMesh::HalfFaceIterator &) const) &easy3d::PolyMesh::HalfFaceIterator::operator==, "are two iterators equal?\n\nC++: easy3d::PolyMesh::HalfFaceIterator::operator==(const class easy3d::PolyMesh::HalfFaceIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::PolyMesh::HalfFaceIterator::*)(const class easy3d::PolyMesh::HalfFaceIterator &) const) &easy3d::PolyMesh::HalfFaceIterator::operator!=, "are two iterators different?\n\nC++: easy3d::PolyMesh::HalfFaceIterator::operator!=(const class easy3d::PolyMesh::HalfFaceIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::PolyMesh::HalfFaceIterator & (easy3d::PolyMesh::HalfFaceIterator::*)()) &easy3d::PolyMesh::HalfFaceIterator::operator++, "pre-increment iterator\n\nC++: easy3d::PolyMesh::HalfFaceIterator::operator++() --> class easy3d::PolyMesh::HalfFaceIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::PolyMesh::HalfFaceIterator & (easy3d::PolyMesh::HalfFaceIterator::*)()) &easy3d::PolyMesh::HalfFaceIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::PolyMesh::HalfFaceIterator::operator--() --> class easy3d::PolyMesh::HalfFaceIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::PolyMesh::FaceIterator file:easy3d/core/poly_mesh.h line:501
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::FaceIterator, std::shared_ptr<easy3d::PolyMesh::FaceIterator>> cl(enclosing_class, "FaceIterator", "this class iterates linearly over all faces\n \n\n faces_begin(), faces_end()\n \n\n VertexIterator, EdgeIterator, HalfFaceIterator, CellIterator");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::FaceIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::PolyMesh::Face const & a0){ return new easy3d::PolyMesh::FaceIterator(a0); } ), "doc" , pybind11::arg("f"));
			cl.def( pybind11::init<struct easy3d::PolyMesh::Face, const class easy3d::PolyMesh *>(), pybind11::arg("f"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::FaceIterator const &o){ return new easy3d::PolyMesh::FaceIterator(o); } ) );
			cl.def("dereference", (struct easy3d::PolyMesh::Face (easy3d::PolyMesh::FaceIterator::*)() const) &easy3d::PolyMesh::FaceIterator::operator*, "get the face the iterator refers to\n\nC++: easy3d::PolyMesh::FaceIterator::operator*() const --> struct easy3d::PolyMesh::Face");
			cl.def("__eq__", (bool (easy3d::PolyMesh::FaceIterator::*)(const class easy3d::PolyMesh::FaceIterator &) const) &easy3d::PolyMesh::FaceIterator::operator==, "are two iterators equal?\n\nC++: easy3d::PolyMesh::FaceIterator::operator==(const class easy3d::PolyMesh::FaceIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::PolyMesh::FaceIterator::*)(const class easy3d::PolyMesh::FaceIterator &) const) &easy3d::PolyMesh::FaceIterator::operator!=, "are two iterators different?\n\nC++: easy3d::PolyMesh::FaceIterator::operator!=(const class easy3d::PolyMesh::FaceIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::PolyMesh::FaceIterator & (easy3d::PolyMesh::FaceIterator::*)()) &easy3d::PolyMesh::FaceIterator::operator++, "pre-increment iterator\n\nC++: easy3d::PolyMesh::FaceIterator::operator++() --> class easy3d::PolyMesh::FaceIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::PolyMesh::FaceIterator & (easy3d::PolyMesh::FaceIterator::*)()) &easy3d::PolyMesh::FaceIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::PolyMesh::FaceIterator::operator--() --> class easy3d::PolyMesh::FaceIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::PolyMesh::CellIterator file:easy3d/core/poly_mesh.h line:550
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::CellIterator, std::shared_ptr<easy3d::PolyMesh::CellIterator>> cl(enclosing_class, "CellIterator", "this class iterates linearly over all cells\n \n\n cells_begin(), cells_end()\n \n\n VertexIterator, EdgeIterator, HalfFaceIterator, FaceIterator");
			cl.def( pybind11::init( [](){ return new easy3d::PolyMesh::CellIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::PolyMesh::Cell const & a0){ return new easy3d::PolyMesh::CellIterator(a0); } ), "doc" , pybind11::arg("c"));
			cl.def( pybind11::init<struct easy3d::PolyMesh::Cell, const class easy3d::PolyMesh *>(), pybind11::arg("c"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::CellIterator const &o){ return new easy3d::PolyMesh::CellIterator(o); } ) );
			cl.def("dereference", (struct easy3d::PolyMesh::Cell (easy3d::PolyMesh::CellIterator::*)() const) &easy3d::PolyMesh::CellIterator::operator*, "get the cell the iterator refers to\n\nC++: easy3d::PolyMesh::CellIterator::operator*() const --> struct easy3d::PolyMesh::Cell");
			cl.def("__eq__", (bool (easy3d::PolyMesh::CellIterator::*)(const class easy3d::PolyMesh::CellIterator &) const) &easy3d::PolyMesh::CellIterator::operator==, "are two iterators equal?\n\nC++: easy3d::PolyMesh::CellIterator::operator==(const class easy3d::PolyMesh::CellIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::PolyMesh::CellIterator::*)(const class easy3d::PolyMesh::CellIterator &) const) &easy3d::PolyMesh::CellIterator::operator!=, "are two iterators different?\n\nC++: easy3d::PolyMesh::CellIterator::operator!=(const class easy3d::PolyMesh::CellIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::PolyMesh::CellIterator & (easy3d::PolyMesh::CellIterator::*)()) &easy3d::PolyMesh::CellIterator::operator++, "pre-increment iterator\n\nC++: easy3d::PolyMesh::CellIterator::operator++() --> class easy3d::PolyMesh::CellIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::PolyMesh::CellIterator & (easy3d::PolyMesh::CellIterator::*)()) &easy3d::PolyMesh::CellIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::PolyMesh::CellIterator::operator--() --> class easy3d::PolyMesh::CellIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::PolyMesh::VertexContainer file:easy3d/core/poly_mesh.h line:601
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::VertexContainer, std::shared_ptr<easy3d::PolyMesh::VertexContainer>> cl(enclosing_class, "VertexContainer", "this helper class is a container for iterating through all\n vertices using C++11 range-based for-loops.\n \n\n vertices()");
			cl.def( pybind11::init<class easy3d::PolyMesh::VertexIterator, class easy3d::PolyMesh::VertexIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::VertexContainer const &o){ return new easy3d::PolyMesh::VertexContainer(o); } ) );
			cl.def("begin", (class easy3d::PolyMesh::VertexIterator (easy3d::PolyMesh::VertexContainer::*)() const) &easy3d::PolyMesh::VertexContainer::begin, "C++: easy3d::PolyMesh::VertexContainer::begin() const --> class easy3d::PolyMesh::VertexIterator");
			cl.def("end", (class easy3d::PolyMesh::VertexIterator (easy3d::PolyMesh::VertexContainer::*)() const) &easy3d::PolyMesh::VertexContainer::end, "C++: easy3d::PolyMesh::VertexContainer::end() const --> class easy3d::PolyMesh::VertexIterator");
		}

		{ // easy3d::PolyMesh::EdgeContainer file:easy3d/core/poly_mesh.h line:615
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::EdgeContainer, std::shared_ptr<easy3d::PolyMesh::EdgeContainer>> cl(enclosing_class, "EdgeContainer", "this helper class is a container for iterating through all\n edges using C++11 range-based for-loops.\n \n\n edges()");
			cl.def( pybind11::init<class easy3d::PolyMesh::EdgeIterator, class easy3d::PolyMesh::EdgeIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::EdgeContainer const &o){ return new easy3d::PolyMesh::EdgeContainer(o); } ) );
			cl.def("begin", (class easy3d::PolyMesh::EdgeIterator (easy3d::PolyMesh::EdgeContainer::*)() const) &easy3d::PolyMesh::EdgeContainer::begin, "C++: easy3d::PolyMesh::EdgeContainer::begin() const --> class easy3d::PolyMesh::EdgeIterator");
			cl.def("end", (class easy3d::PolyMesh::EdgeIterator (easy3d::PolyMesh::EdgeContainer::*)() const) &easy3d::PolyMesh::EdgeContainer::end, "C++: easy3d::PolyMesh::EdgeContainer::end() const --> class easy3d::PolyMesh::EdgeIterator");
		}

		{ // easy3d::PolyMesh::HalffaceContainer file:easy3d/core/poly_mesh.h line:629
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::HalffaceContainer, std::shared_ptr<easy3d::PolyMesh::HalffaceContainer>> cl(enclosing_class, "HalffaceContainer", "this helper class is a container for iterating through all\n halffaces using C++11 range-based for-loops.\n \n\n halffaces()");
			cl.def( pybind11::init<class easy3d::PolyMesh::HalfFaceIterator, class easy3d::PolyMesh::HalfFaceIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::HalffaceContainer const &o){ return new easy3d::PolyMesh::HalffaceContainer(o); } ) );
			cl.def("begin", (class easy3d::PolyMesh::HalfFaceIterator (easy3d::PolyMesh::HalffaceContainer::*)() const) &easy3d::PolyMesh::HalffaceContainer::begin, "C++: easy3d::PolyMesh::HalffaceContainer::begin() const --> class easy3d::PolyMesh::HalfFaceIterator");
			cl.def("end", (class easy3d::PolyMesh::HalfFaceIterator (easy3d::PolyMesh::HalffaceContainer::*)() const) &easy3d::PolyMesh::HalffaceContainer::end, "C++: easy3d::PolyMesh::HalffaceContainer::end() const --> class easy3d::PolyMesh::HalfFaceIterator");
		}

		{ // easy3d::PolyMesh::FaceContainer file:easy3d/core/poly_mesh.h line:643
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::FaceContainer, std::shared_ptr<easy3d::PolyMesh::FaceContainer>> cl(enclosing_class, "FaceContainer", "this helper class is a container for iterating through all\n faces using C++11 range-based for-loops.\n \n\n faces()");
			cl.def( pybind11::init<class easy3d::PolyMesh::FaceIterator, class easy3d::PolyMesh::FaceIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::FaceContainer const &o){ return new easy3d::PolyMesh::FaceContainer(o); } ) );
			cl.def("begin", (class easy3d::PolyMesh::FaceIterator (easy3d::PolyMesh::FaceContainer::*)() const) &easy3d::PolyMesh::FaceContainer::begin, "C++: easy3d::PolyMesh::FaceContainer::begin() const --> class easy3d::PolyMesh::FaceIterator");
			cl.def("end", (class easy3d::PolyMesh::FaceIterator (easy3d::PolyMesh::FaceContainer::*)() const) &easy3d::PolyMesh::FaceContainer::end, "C++: easy3d::PolyMesh::FaceContainer::end() const --> class easy3d::PolyMesh::FaceIterator");
		}

		{ // easy3d::PolyMesh::CellContainer file:easy3d/core/poly_mesh.h line:657
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PolyMesh::CellContainer, std::shared_ptr<easy3d::PolyMesh::CellContainer>> cl(enclosing_class, "CellContainer", "this helper class is a container for iterating through all\n cells using C++11 range-based for-loops.\n \n\n cells()");
			cl.def( pybind11::init<class easy3d::PolyMesh::CellIterator, class easy3d::PolyMesh::CellIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::PolyMesh::CellContainer const &o){ return new easy3d::PolyMesh::CellContainer(o); } ) );
			cl.def("begin", (class easy3d::PolyMesh::CellIterator (easy3d::PolyMesh::CellContainer::*)() const) &easy3d::PolyMesh::CellContainer::begin, "C++: easy3d::PolyMesh::CellContainer::begin() const --> class easy3d::PolyMesh::CellIterator");
			cl.def("end", (class easy3d::PolyMesh::CellIterator (easy3d::PolyMesh::CellContainer::*)() const) &easy3d::PolyMesh::CellContainer::end, "C++: easy3d::PolyMesh::CellContainer::end() const --> class easy3d::PolyMesh::CellIterator");
		}
	}

    { // easy3d::PropertyArray file:easy3d/core/property.h line:110
        pybind11::class_<easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_HalfFaceConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_PolyMesh_HalfFaceConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_HalfFaceConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::PolyMesh::HalfFaceConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_HalfFaceConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_HalfFaceConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::PolyMesh::HalfFaceConnectivity * (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::data() const --> const struct easy3d::PolyMesh::HalfFaceConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::PolyMesh::HalfFaceConnectivity & (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::HalfFaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> &)) &easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::PolyMesh::HalfFaceConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::PolyMesh::HalfFaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
        pybind11::class_<easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_CellConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_PolyMesh_CellConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_CellConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::PolyMesh::CellConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_CellConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_CellConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::PolyMesh::CellConnectivity * (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::data() const --> const struct easy3d::PolyMesh::CellConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::PolyMesh::CellConnectivity & (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::CellConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> &)) &easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::PolyMesh::CellConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::PolyMesh::CellConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
        pybind11::class_<easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_VertexConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_PolyMesh_VertexConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_VertexConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::PolyMesh::VertexConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_VertexConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::PolyMesh::VertexConnectivity * (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::data() const --> const struct easy3d::PolyMesh::VertexConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::PolyMesh::VertexConnectivity & (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> &)) &easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::PolyMesh::VertexConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::PolyMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
        pybind11::class_<easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_EdgeConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_PolyMesh_EdgeConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_EdgeConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::PolyMesh::EdgeConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_EdgeConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_PolyMesh_EdgeConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::PolyMesh::EdgeConnectivity * (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::data() const --> const struct easy3d::PolyMesh::EdgeConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::vector() --> class std::vector<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::PolyMesh::EdgeConnectivity & (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::operator[](size_t) --> struct easy3d::PolyMesh::EdgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> & (easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> &)) &easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::PolyMesh::EdgeConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::PolyMesh::EdgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
