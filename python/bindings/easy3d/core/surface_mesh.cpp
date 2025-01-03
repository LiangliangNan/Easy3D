#include <easy3d/core/vec.h>
#include <easy3d/core/property.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>

#include <memory>
#include <typeinfo>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Halfedge_t : public easy3d::Property<easy3d::SurfaceMesh::Halfedge> {
    using easy3d::Property<easy3d::SurfaceMesh::Halfedge>::Property;

    struct easy3d::SurfaceMesh::Halfedge & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::SurfaceMesh::Halfedge> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::Halfedge &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::Halfedge &> caster;
                return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::Halfedge &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::Halfedge &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_SurfaceMesh_VertexConnectivity_t : public easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity> {
    using easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::Property;

    struct easy3d::SurfaceMesh::VertexConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::VertexConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::VertexConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::VertexConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::VertexConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_SurfaceMesh_HalfedgeConnectivity_t : public easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity> {
    using easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::Property;

    struct easy3d::SurfaceMesh::HalfedgeConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::HalfedgeConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::HalfedgeConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::HalfedgeConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::HalfedgeConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_SurfaceMesh_FaceConnectivity_t : public easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity> {
    using easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::Property;

    struct easy3d::SurfaceMesh::FaceConnectivity & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::FaceConnectivity &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::FaceConnectivity &> caster;
                return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::FaceConnectivity &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::FaceConnectivity &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};


// easy3d::SurfaceMesh file:easy3d/core/surface_mesh.h line:51
struct PyCallBack_easy3d_SurfaceMesh : public easy3d::SurfaceMesh {
	using easy3d::SurfaceMesh::SurfaceMesh;

	void property_stats(std::ostream & a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh *>(this), "property_stats");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return SurfaceMesh::property_stats(a0);
	}
	using _binder_ret_0 = std::vector<class easy3d::Vec<3, float> > &;
	_binder_ret_0 points() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh *>(this), "points");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<_binder_ret_0>::value) {
				static pybind11::detail::override_caster_t<_binder_ret_0> caster;
				return pybind11::detail::cast_ref<_binder_ret_0>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<_binder_ret_0>(std::move(o));
		}
		return SurfaceMesh::points();
	}
};

// easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
struct PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_SurfaceMesh_VertexConnectivity_t : public easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity> {
	using easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::VertexProperty;

	struct easy3d::SurfaceMesh::VertexConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::VertexConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::VertexConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::VertexConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::VertexConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};

// easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
struct PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_Vec_3_float_t : public easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>> {
	using easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::VertexProperty;

	using _binder_ret_0 = easy3d::Vec<3, float> &;
	_binder_ret_0 operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>> *>(this), "__getitem__");
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

// easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
struct PyCallBack_easy3d_SurfaceMesh_VertexProperty_float_t : public easy3d::SurfaceMesh::VertexProperty<float> {
	using easy3d::SurfaceMesh::VertexProperty<float>::VertexProperty;

	float & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::VertexProperty<float> *>(this), "__getitem__");
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

// easy3d::SurfaceMesh::HalfedgeProperty file:easy3d/core/surface_mesh.h line:208
struct PyCallBack_easy3d_SurfaceMesh_HalfedgeProperty_easy3d_SurfaceMesh_HalfedgeConnectivity_t : public easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity> {
	using easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::HalfedgeProperty;

	struct easy3d::SurfaceMesh::HalfedgeConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::HalfedgeConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::HalfedgeConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::HalfedgeConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::HalfedgeConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};

// easy3d::SurfaceMesh::FaceProperty file:easy3d/core/surface_mesh.h line:256
struct PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_SurfaceMesh_FaceConnectivity_t : public easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity> {
	using easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::FaceProperty;

	struct easy3d::SurfaceMesh::FaceConnectivity & operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "__getitem__");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::FaceConnectivity &>::value) {
				static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::FaceConnectivity &> caster;
				return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::FaceConnectivity &>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::FaceConnectivity &>(std::move(o));
		}
		return Property::operator[](a0);
	}
};


// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_VertexConnectivity_t : public easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> {
    using easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> *>(this), "type");
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
struct PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_HalfedgeConnectivity_t : public easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> {
    using easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> *>(this), "type");
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

// easy3d::SurfaceMesh::FaceProperty file:easy3d/core/surface_mesh.h line:256
struct PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_Vec_3_float_t : public easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>> {
	using easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::FaceProperty;

	using _binder_ret_0 = easy3d::Vec<3, float> &;
	_binder_ret_0 operator[](size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>> *>(this), "__getitem__");
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

// easy3d::Property file:easy3d/core/property.h line:252
struct PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Vertex_t : public easy3d::Property<easy3d::SurfaceMesh::Vertex> {
    using easy3d::Property<easy3d::SurfaceMesh::Vertex>::Property;

    struct easy3d::SurfaceMesh::Vertex & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::SurfaceMesh::Vertex> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::Vertex &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::Vertex &> caster;
                return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::Vertex &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::Vertex &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};


// easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
struct PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_SurfaceMesh_Vertex_t : public easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::Vertex> {
    using easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::Vertex>::VertexProperty;

    struct easy3d::SurfaceMesh::Vertex & operator[](size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::Vertex> *>(this), "__getitem__");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<struct easy3d::SurfaceMesh::Vertex &>::value) {
                static pybind11::detail::override_caster_t<struct easy3d::SurfaceMesh::Vertex &> caster;
                return pybind11::detail::cast_ref<struct easy3d::SurfaceMesh::Vertex &>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<struct easy3d::SurfaceMesh::Vertex &>(std::move(o));
        }
        return Property::operator[](a0);
    }
};

// easy3d::PropertyArray file:easy3d/core/property.h line:110
struct PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_Vertex_t : public easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> {
    using easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> *>(this), "type");
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
struct PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_FaceConnectivity_t : public easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> {
    using easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::PropertyArray;

    void reserve(size_t a0) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "reserve");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "resize");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "push_back");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "reset");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "transfer");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "shrink_to_fit");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "swap");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "copy");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "empty_clone");
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
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> *>(this), "type");
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


// Convert either Python's list of lists (native Python structure) or 1D NumPy arrays of objects (dtype=object)
// to std::vector< std::vector<unsigned int> >.
// The "input" represent the vertex indices of the faces in a general polygonal mesh. The total number of elements
// in the input array is equal to the total number of faces in the mesh, and the i-th element has the number of
// unsigned int type of indices for the i-th face.
std::vector<std::vector<unsigned int>> convert(const pybind11::object& input) {
    std::vector<std::vector<unsigned int>> result;

    // Check if the input is a Python list
    if (pybind11::isinstance<pybind11::list>(input)) {
        // Convert the input to a Python list of lists
        pybind11::list input_list = pybind11::cast<pybind11::list>(input);
        for (ssize_t i = 0; i < input_list.size(); ++i) {
            pybind11::object face = input_list[i];
            if (pybind11::isinstance<pybind11::list>(face)) {
                pybind11::list face_list = pybind11::cast<pybind11::list>(face);
                std::vector<unsigned int> face_indices;
                // Convert each element of the face list to unsigned int
                for (ssize_t j = 0; j < face_list.size(); ++j) {
                    face_indices.push_back(static_cast<unsigned int>(pybind11::cast<long>(face_list[j])));
                }
                result.push_back(std::move(face_indices));
            } else {
                // Handle case where face is not a list (error or unexpected data)
                throw std::runtime_error("Expected a list of lists for vertex indices");
            }
        }
    }
    // Check if the input is a 1D NumPy array of objects (dtype=object)
    else if (pybind11::isinstance<pybind11::array>(input)) {
        auto arr = pybind11::cast<pybind11::array>(input);
        // Check if it's a 1D array of Python objects
        if (arr.ndim() == 1 && arr.dtype().is(pybind11::dtype::of<pybind11::object>())) {
            auto buf = arr.unchecked<pybind11::object>();  // 1D access
            for (ssize_t i = 0; i < arr.shape(0); ++i) {
                pybind11::object face = buf(i);
                if (pybind11::isinstance<pybind11::list>(face)) {
                    pybind11::list face_list = pybind11::cast<pybind11::list>(face);
                    std::vector<unsigned int> face_indices;
                    // Convert each element of the face list to unsigned int
                    for (ssize_t j = 0; j < face_list.size(); ++j) {
                        face_indices.push_back(static_cast<unsigned int>(pybind11::cast<long>(face_list[j])));
                    }
                    result.push_back(std::move(face_indices));
                } else {
                    // Handle case where face is not a list (error or unexpected data)
                    throw std::runtime_error("Expected a list of lists for vertex indices");
                }
            }
        } else {
            throw std::runtime_error("Expected a 1D NumPy array with dtype=object");
        }
    } else {
        throw std::runtime_error("Expected a Python list or 1D NumPy array with dtype=object");
    }

    return result;
}



void bind_easy3d_core_surface_mesh(pybind11::module_& m)
{
	{ // easy3d::SurfaceMesh file:easy3d/core/surface_mesh.h line:51
		pybind11::class_<easy3d::SurfaceMesh, std::shared_ptr<easy3d::SurfaceMesh>, PyCallBack_easy3d_SurfaceMesh, easy3d::Model> cl(m, "SurfaceMesh", "A halfedge data structure for polygonal meshes of 2-manifold.\n\n \n A surface mesh is a consistent and orientable polygonal mesh that may have one or more boundaries.\n      The faces are simple polygons and the edges are line segments. Each edge connects two vertices, and is\n      shared by two faces (including the null face for boundary edges). A surface mesh can have any number\n      of connected components and also some self-intersections.\n\n \n The construction of a manifold surface mesh can be done by iteratively calling add_vertex() and\n      add_face(). These two methods can ONLY be used when you're sure that the mesh is manifold. Otherwise,\n      SurfaceMeshBuilder should be used for the construction, which guarantees you end up with a polygonal\n      mesh of a 2-manifold topology. In any case, client code is highly recommended to use SurfaceMeshBuilder.\n\n \n\n \n SurfaceMeshBuilder.");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_SurfaceMesh const &o){ return new PyCallBack_easy3d_SurfaceMesh(o); } ) );
		cl.def( pybind11::init( [](easy3d::SurfaceMesh const &o){ return new easy3d::SurfaceMesh(o); } ) );

        // Initialize SurfaceMesh from vertices and vertex indices of faces.
        //  - the "points" provides the vertex positions. It must be a list of tuples of 3 or a NumPy array with
        //    shape (n, 3);
        //  - the "indices" represents the vertex indices of the faces in a general polygonal mesh. The indices are
        //    0-based (i.e., starting from 0), and must be a list of lists (native Python structure) or a 1D NumPy
        //    array of objects (dtype=object). The total number of elements in "indices" is equal to the total number
        //    of faces in the mesh, and it's i-th element has the number of indices for the i-th face.
        cl.def(pybind11::init([](pybind11::object points, pybind11::object &indices) {
               auto mesh = std::make_shared<easy3d::SurfaceMesh>();

               // First: add the vertices
               if (pybind11::isinstance<pybind11::list>(points)) {
                   // Handle list of tuples
                   for (auto item: points.cast<pybind11::list>()) {
                       auto tuple = item.cast<pybind11::tuple>();
                       if (tuple.size() != 3) {
                           throw std::invalid_argument("Each point must have 3 coordinates.");
                       }
                       mesh->add_vertex(easy3d::vec3(
                               tuple[0].cast<float>(),
                               tuple[1].cast<float>(),
                               tuple[2].cast<float>()
                       ));
                   }
               } else if (pybind11::isinstance<pybind11::array_t<float>>(points)) {    // float type
                   // Handle NumPy array
                   auto arr = points.cast<pybind11::array_t<float>>();
                   if (arr.ndim() != 2 || arr.shape(1) != 3) {
                       throw std::invalid_argument("Input array must have shape (n, 3).");
                   }
                   auto buf = arr.unchecked<2>();
                   for (ssize_t i = 0; i < arr.shape(0); ++i) {
                       mesh->add_vertex(easy3d::vec3(buf(i, 0), buf(i, 1), buf(i, 2)));
                   }
               } else if (pybind11::isinstance<pybind11::array_t<double>>(points)) {    // double type
                   // Handle NumPy array
                   auto arr = points.cast<pybind11::array_t<double>>();
                   if (arr.ndim() != 2 || arr.shape(1) != 3) {
                       throw std::invalid_argument("Input array must have shape (n, 3).");
                   }
                   auto buf = arr.unchecked<2>();
                   for (ssize_t i = 0; i < arr.shape(0); ++i) {
                       mesh->add_vertex(easy3d::vec3(
                               static_cast<float>(buf(i, 0)),
                               static_cast<float>(buf(i, 1)),
                               static_cast<float>(buf(i, 2)))
                       );
                   }
               } else {
                   throw std::invalid_argument(
                           "Input vertices must be a list of tuples or a NumPy array with shape (n, 3).");
               }

               // Second: add the faces
               const auto processed_indices = convert(indices);
               for (const auto &ids: processed_indices) {
                   std::vector<easy3d::SurfaceMesh::Vertex> face;
                   for (const auto id: ids)
                       face.push_back(easy3d::SurfaceMesh::Vertex(id));
                   mesh->add_face(face);
               }
               return mesh;
           }),
           R"doc(
                Initialize a SurfaceMesh (not necessarily a triangle mesh) from vertices and vertex indices of faces.
                - The "points" provides the vertex positions. It must be a list of tuples of 3 or a NumPy array with
                  shape (n, 3);
                - The "indices" represents the vertex indices of the faces in a general polygonal mesh. The indices are
                  0-based (i.e., starting from 0) and must be a list of lists (native Python structure) or a 1D NumPy
                  array of objects (dtype=object). The total number of elements in "indices" is equal to the total number
                  of faces in the mesh, and it's i-th element contains the vertex indices for the i-th face.
                )doc"
        );

        cl.def("name", [](easy3d::SurfaceMesh& self) { return self.name(); }, pybind11::return_value_policy::copy, "Get the name of the surface mesh.");
        cl.def("set_name", [](easy3d::SurfaceMesh& self, const std::string& name) { self.set_name(name); }, "Set the name of the surface mesh.");

        cl.def("renderer", [](const easy3d::SurfaceMesh& self) -> const easy3d::Renderer* { return self.renderer(); }, pybind11::return_value_policy::reference_internal, "Returns the renderer of the model");
        cl.def("renderer", [](easy3d::SurfaceMesh& self) -> easy3d::Renderer* { return self.renderer(); }, pybind11::return_value_policy::reference_internal, "Returns the renderer of the model");

        cl.def("assign", (class easy3d::SurfaceMesh & (easy3d::SurfaceMesh::*)(const class easy3d::SurfaceMesh &)) &easy3d::SurfaceMesh::operator=, "assign  to  performs a deep copy of all properties.\n\nC++: easy3d::SurfaceMesh::operator=(const class easy3d::SurfaceMesh &) --> class easy3d::SurfaceMesh &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__iadd__", (class easy3d::SurfaceMesh & (easy3d::SurfaceMesh::*)(const class easy3d::SurfaceMesh &)) &easy3d::SurfaceMesh::operator+=, "Merges another surface mesh into the current one.\n Shifts the indices of vertices of the other mesh by `number_of_vertices() + number_of_removed_vertices()`\n and analogously for halfedges, edges, and faces.\n Copies entries of all property maps which have the same name in both meshes. That is, properties maps which\n are only in `other` are ignored.\n Also copies elements which are marked as removed, and concatenates the freelists of both meshes.\n\nC++: easy3d::SurfaceMesh::operator+=(const class easy3d::SurfaceMesh &) --> class easy3d::SurfaceMesh &", pybind11::return_value_policy::automatic, pybind11::arg("other"));
		cl.def("join", (class easy3d::SurfaceMesh & (easy3d::SurfaceMesh::*)(const class easy3d::SurfaceMesh &)) &easy3d::SurfaceMesh::join, "Merges another surface mesh into the current one.\n Shifts the indices of vertices of the other mesh by `number_of_vertices() + number_of_removed_vertices()`\n and analogously for halfedges, edges, and faces.\n Copies entries of all property maps which have the same name in both meshes. That is, properties maps which\n are only in `other` are ignored.\n Also copies elements which are marked as removed, and concatenates the freelists of both meshes.\n\nC++: easy3d::SurfaceMesh::join(const class easy3d::SurfaceMesh &) --> class easy3d::SurfaceMesh &", pybind11::return_value_policy::automatic, pybind11::arg("other"));
		cl.def("assign", (class easy3d::SurfaceMesh & (easy3d::SurfaceMesh::*)(const class easy3d::SurfaceMesh &)) &easy3d::SurfaceMesh::assign, "assign  to  does not copy custom properties.\n\nC++: easy3d::SurfaceMesh::assign(const class easy3d::SurfaceMesh &) --> class easy3d::SurfaceMesh &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("read", (bool (easy3d::SurfaceMesh::*)(const std::string &)) &easy3d::SurfaceMesh::read, "Read mesh from a SM file \n Mainly for quick debug purposes. Client code should use SurfaceMeshIO.\n \n\n SurfaceMeshIO.\n\nC++: easy3d::SurfaceMesh::read(const std::string &) --> bool", pybind11::arg("filename"));
		cl.def("write", (bool (easy3d::SurfaceMesh::*)(const std::string &) const) &easy3d::SurfaceMesh::write, "Write mesh to a SM file \n Mainly for quick debug purposes. Client code should use SurfaceMeshIO.\n \n\n SurfaceMeshIO.\n\nC++: easy3d::SurfaceMesh::write(const std::string &) const --> bool", pybind11::arg("filename"));
		cl.def("add_vertex", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::*)(const class easy3d::Vec<3, float> &)) &easy3d::SurfaceMesh::add_vertex, "add a new vertex with position \n\nC++: easy3d::SurfaceMesh::add_vertex(const class easy3d::Vec<3, float> &) --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("p"));
		cl.def("add_face", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::*)(const class std::vector<struct easy3d::SurfaceMesh::Vertex> &)) &easy3d::SurfaceMesh::add_face, "add a new face with vertex list \n \n\n The input vertices created by add_vertex().\n \n\n add_triangle, add_quad\n\nC++: easy3d::SurfaceMesh::add_face(const class std::vector<struct easy3d::SurfaceMesh::Vertex> &) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("vertices"));
		cl.def("add_triangle", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::add_triangle, "add a new triangle connecting vertices   \n \n\n v2, v3} The input vertices created by add_vertex().\n \n\n add_face, add_quad\n\nC++: easy3d::SurfaceMesh::add_triangle(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"));
		cl.def("add_quad", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::add_quad, "add a new quad connecting vertices    \n \n\n v2, v3, v4} The input vertices created by add_vertex().\n \n\n add_triangle, add_face\n\nC++: easy3d::SurfaceMesh::add_quad(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"), pybind11::arg("v4"));
		cl.def("vertices_size", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::vertices_size, "returns number of (deleted and valid) vertices in the mesh\n\nC++: easy3d::SurfaceMesh::vertices_size() const --> unsigned int");
		cl.def("halfedges_size", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::halfedges_size, "returns number of (deleted and valid)halfedge in the mesh\n\nC++: easy3d::SurfaceMesh::halfedges_size() const --> unsigned int");
		cl.def("edges_size", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::edges_size, "returns number of (deleted and valid)edges in the mesh\n\nC++: easy3d::SurfaceMesh::edges_size() const --> unsigned int");
		cl.def("faces_size", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::faces_size, "returns number of (deleted and valid)faces in the mesh\n\nC++: easy3d::SurfaceMesh::faces_size() const --> unsigned int");
		cl.def("n_vertices", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::n_vertices, "returns number of vertices in the mesh\n\nC++: easy3d::SurfaceMesh::n_vertices() const --> unsigned int");
		cl.def("n_halfedges", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::n_halfedges, "returns number of halfedge in the mesh\n\nC++: easy3d::SurfaceMesh::n_halfedges() const --> unsigned int");
		cl.def("n_edges", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::n_edges, "returns number of edges in the mesh\n\nC++: easy3d::SurfaceMesh::n_edges() const --> unsigned int");
		cl.def("n_faces", (unsigned int (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::n_faces, "returns number of faces in the mesh\n\nC++: easy3d::SurfaceMesh::n_faces() const --> unsigned int");
		cl.def("clear", (void (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::clear, "Removes all vertices, edges, faces, and properties (and resets garbage state).\n \n\n After calling this method, the mesh is the same as newly constructed. The additional properties\n (such as normal vectors) are also removed and must thus be re-added if needed.\n\nC++: easy3d::SurfaceMesh::clear() --> void");
		cl.def("reserve", (void (easy3d::SurfaceMesh::*)(unsigned int, unsigned int, unsigned int)) &easy3d::SurfaceMesh::reserve, "reserves memory (mainly used in file readers)\n\nC++: easy3d::SurfaceMesh::reserve(unsigned int, unsigned int, unsigned int) --> void", pybind11::arg("nvertices"), pybind11::arg("nedges"), pybind11::arg("nfaces"));
		cl.def("resize", (void (easy3d::SurfaceMesh::*)(unsigned int, unsigned int, unsigned int)) &easy3d::SurfaceMesh::resize, "resizes space for vertices, halfedges, edges, faces, and their currently\n associated properties.\n Note: ne is the number of edges. for halfedges, nh = 2 * ne. */\n\nC++: easy3d::SurfaceMesh::resize(unsigned int, unsigned int, unsigned int) --> void", pybind11::arg("nv"), pybind11::arg("ne"), pybind11::arg("nf"));
		cl.def("has_garbage", (bool (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::has_garbage, "are there deleted vertices, edges or faces?\n\nC++: easy3d::SurfaceMesh::has_garbage() const --> bool");
		cl.def("collect_garbage", (void (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::collect_garbage, "remove deleted vertices/edges/faces\n\nC++: easy3d::SurfaceMesh::collect_garbage() --> void");
		cl.def("is_deleted", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::is_deleted, "returns whether vertex  is deleted\n \n\n collect_garbage()\n\nC++: easy3d::SurfaceMesh::is_deleted(struct easy3d::SurfaceMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_deleted", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::is_deleted, "returns whether halfedge  is deleted\n \n\n collect_garbage()\n\nC++: easy3d::SurfaceMesh::is_deleted(struct easy3d::SurfaceMesh::Halfedge) const --> bool", pybind11::arg("h"));
		cl.def("is_deleted", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge) const) &easy3d::SurfaceMesh::is_deleted, "returns whether edge  is deleted\n \n\n collect_garbage()\n\nC++: easy3d::SurfaceMesh::is_deleted(struct easy3d::SurfaceMesh::Edge) const --> bool", pybind11::arg("e"));
		cl.def("is_deleted", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::is_deleted, "returns whether face  is deleted\n \n\n collect_garbage()\n\nC++: easy3d::SurfaceMesh::is_deleted(struct easy3d::SurfaceMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("is_valid", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::is_valid, "return whether vertex  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::SurfaceMesh::is_valid(struct easy3d::SurfaceMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_valid", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::is_valid, "return whether halfedge  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::SurfaceMesh::is_valid(struct easy3d::SurfaceMesh::Halfedge) const --> bool", pybind11::arg("h"));
		cl.def("is_valid", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge) const) &easy3d::SurfaceMesh::is_valid, "return whether edge  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::SurfaceMesh::is_valid(struct easy3d::SurfaceMesh::Edge) const --> bool", pybind11::arg("e"));
		cl.def("is_valid", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::is_valid, "return whether face  is valid, i.e. the index is stores it within the array bounds.\n\nC++: easy3d::SurfaceMesh::is_valid(struct easy3d::SurfaceMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("out_halfedge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::out_halfedge, "returns an outgoing halfedge of vertex \n if  is a boundary vertex this will be a boundary halfedge.\n\nC++: easy3d::SurfaceMesh::out_halfedge(struct easy3d::SurfaceMesh::Vertex) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("v"));
		cl.def("set_out_halfedge", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::set_out_halfedge, "set the outgoing halfedge of vertex  to \n\nC++: easy3d::SurfaceMesh::set_out_halfedge(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Halfedge) --> void", pybind11::arg("v"), pybind11::arg("h"));
		cl.def("is_border", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::is_border, "returns whether  is a boundary vertex\n\nC++: easy3d::SurfaceMesh::is_border(struct easy3d::SurfaceMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_isolated", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::is_isolated, "returns whether  is isolated, i.e., not incident to any face\n\nC++: easy3d::SurfaceMesh::is_isolated(struct easy3d::SurfaceMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_manifold", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::is_manifold, "returns whether  is a manifold vertex (not incident to several patches)\n\nC++: easy3d::SurfaceMesh::is_manifold(struct easy3d::SurfaceMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("is_degenerate", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::is_degenerate, "returns whether  is degenerate\n\nC++: easy3d::SurfaceMesh::is_degenerate(struct easy3d::SurfaceMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("target", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::target, "returns the vertex the halfedge  points to\n\nC++: easy3d::SurfaceMesh::target(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("h"));
		cl.def("source", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::source, "returns the vertex the halfedge  emanates from\n\nC++: easy3d::SurfaceMesh::source(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("h"));
		cl.def("set_target", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::set_target, "sets the vertex the halfedge  points to to \n\nC++: easy3d::SurfaceMesh::set_target(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Vertex) --> void", pybind11::arg("h"), pybind11::arg("v"));
		cl.def("face", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::face, "returns the face incident to halfedge \n\nC++: easy3d::SurfaceMesh::face(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Face", pybind11::arg("h"));
		cl.def("set_face", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Face)) &easy3d::SurfaceMesh::set_face, "sets the incident face to halfedge  to \n\nC++: easy3d::SurfaceMesh::set_face(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Face) --> void", pybind11::arg("h"), pybind11::arg("f"));
		cl.def("next", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::next, "returns the next halfedge within the incident face\n\nC++: easy3d::SurfaceMesh::next(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("set_next", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::set_next, "sets the next halfedge of  within the face to \n\nC++: easy3d::SurfaceMesh::set_next(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge) --> void", pybind11::arg("h"), pybind11::arg("nh"));
		cl.def("prev", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::prev, "returns the previous halfedge within the incident face\n\nC++: easy3d::SurfaceMesh::prev(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("opposite", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::opposite, "returns the opposite halfedge of \n\nC++: easy3d::SurfaceMesh::opposite(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("prev_around_source", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::prev_around_source, "returns the halfedge that is rotated  around the\n start vertex of  it is the opposite halfedge of the previous halfedge of \n\nC++: easy3d::SurfaceMesh::prev_around_source(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("next_around_source", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::next_around_source, "returns the halfedge that is rotated  around the\n start vertex of  it is the next halfedge of the opposite halfedge of \n\nC++: easy3d::SurfaceMesh::next_around_source(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("prev_around_target", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::prev_around_target, "returns the halfedge that is rotated  around the\n end vertex of  it is the prev halfedge of the opposite halfedge of \n\nC++: easy3d::SurfaceMesh::prev_around_target(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("next_around_target", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::next_around_target, "returns the halfedge that is rotated  around the\n end vertex of  it is the opposite halfedge of the next halfedge of \n\nC++: easy3d::SurfaceMesh::next_around_target(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"));
		cl.def("edge", (struct easy3d::SurfaceMesh::Edge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::edge, "return the edge that contains halfedge  as one of its two halfedges.\n\nC++: easy3d::SurfaceMesh::edge(struct easy3d::SurfaceMesh::Halfedge) const --> struct easy3d::SurfaceMesh::Edge", pybind11::arg("h"));
		cl.def("is_border", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::is_border, "returns whether h is a boundary halfedge, i.e., if its face does not exist.\n\nC++: easy3d::SurfaceMesh::is_border(struct easy3d::SurfaceMesh::Halfedge) const --> bool", pybind11::arg("h"));
		cl.def("halfedge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, unsigned int) const) &easy3d::SurfaceMesh::halfedge, "returns the  halfedge of edge   has to be 0 or 1.\n\nC++: easy3d::SurfaceMesh::halfedge(struct easy3d::SurfaceMesh::Edge, unsigned int) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("e"), pybind11::arg("i"));
		cl.def("vertex", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, unsigned int) const) &easy3d::SurfaceMesh::vertex, "returns the  vertex of edge   has to be 0 or 1.\n\nC++: easy3d::SurfaceMesh::vertex(struct easy3d::SurfaceMesh::Edge, unsigned int) const --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("e"), pybind11::arg("i"));
		cl.def("face", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, unsigned int) const) &easy3d::SurfaceMesh::face, "returns the face incident to the  halfedge of edge   has to be 0 or 1.\n\nC++: easy3d::SurfaceMesh::face(struct easy3d::SurfaceMesh::Edge, unsigned int) const --> struct easy3d::SurfaceMesh::Face", pybind11::arg("e"), pybind11::arg("i"));
		cl.def("is_border", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge) const) &easy3d::SurfaceMesh::is_border, "returns whether  is a boundary edge, i.e., if one of its\n halfedges is a boundary halfedge.\n\nC++: easy3d::SurfaceMesh::is_border(struct easy3d::SurfaceMesh::Edge) const --> bool", pybind11::arg("e"));
		cl.def("halfedge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::halfedge, "returns a halfedge of face \n\nC++: easy3d::SurfaceMesh::halfedge(struct easy3d::SurfaceMesh::Face) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("f"));
		cl.def("set_halfedge", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face, struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::set_halfedge, "sets the halfedge of face  to \n\nC++: easy3d::SurfaceMesh::set_halfedge(struct easy3d::SurfaceMesh::Face, struct easy3d::SurfaceMesh::Halfedge) --> void", pybind11::arg("f"), pybind11::arg("h"));
		cl.def("is_border", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::is_border, "returns whether  is a boundary face, i.e., it one of its edges is a boundary edge.\n\nC++: easy3d::SurfaceMesh::is_border(struct easy3d::SurfaceMesh::Face) const --> bool", pybind11::arg("f"));
		cl.def("remove_vertex_property", (bool (easy3d::SurfaceMesh::*)(const std::string &)) &easy3d::SurfaceMesh::remove_vertex_property, "remove the vertex property named \n\nC++: easy3d::SurfaceMesh::remove_vertex_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_halfedge_property", (bool (easy3d::SurfaceMesh::*)(const std::string &)) &easy3d::SurfaceMesh::remove_halfedge_property, "remove the halfedge property named \n\nC++: easy3d::SurfaceMesh::remove_halfedge_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_edge_property", (bool (easy3d::SurfaceMesh::*)(const std::string &)) &easy3d::SurfaceMesh::remove_edge_property, "remove the edge property named \n\nC++: easy3d::SurfaceMesh::remove_edge_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_face_property", (bool (easy3d::SurfaceMesh::*)(const std::string &)) &easy3d::SurfaceMesh::remove_face_property, "remove the face property named \n\nC++: easy3d::SurfaceMesh::remove_face_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("remove_model_property", (bool (easy3d::SurfaceMesh::*)(const std::string &)) &easy3d::SurfaceMesh::remove_model_property, "remove the model property named \n\nC++: easy3d::SurfaceMesh::remove_model_property(const std::string &) --> bool", pybind11::arg("n"));
		cl.def("rename_vertex_property", (bool (easy3d::SurfaceMesh::*)(const std::string &, const std::string &)) &easy3d::SurfaceMesh::rename_vertex_property, "rename a vertex property given its name\n\nC++: easy3d::SurfaceMesh::rename_vertex_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_face_property", (bool (easy3d::SurfaceMesh::*)(const std::string &, const std::string &)) &easy3d::SurfaceMesh::rename_face_property, "rename a face property given its name\n\nC++: easy3d::SurfaceMesh::rename_face_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_edge_property", (bool (easy3d::SurfaceMesh::*)(const std::string &, const std::string &)) &easy3d::SurfaceMesh::rename_edge_property, "rename an edge property given its name\n\nC++: easy3d::SurfaceMesh::rename_edge_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_halfedge_property", (bool (easy3d::SurfaceMesh::*)(const std::string &, const std::string &)) &easy3d::SurfaceMesh::rename_halfedge_property, "rename a halfedge property given its name\n\nC++: easy3d::SurfaceMesh::rename_halfedge_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("rename_model_property", (bool (easy3d::SurfaceMesh::*)(const std::string &, const std::string &)) &easy3d::SurfaceMesh::rename_model_property, "rename a model property given its name\n\nC++: easy3d::SurfaceMesh::rename_model_property(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));
		cl.def("get_vertex_property_type", (const class std::type_info & (easy3d::SurfaceMesh::*)(const std::string &) const) &easy3d::SurfaceMesh::get_vertex_property_type, "get the type_info  of vertex property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::SurfaceMesh::get_vertex_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_halfedge_property_type", (const class std::type_info & (easy3d::SurfaceMesh::*)(const std::string &) const) &easy3d::SurfaceMesh::get_halfedge_property_type, "get the type_info  of halfedge property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::SurfaceMesh::get_halfedge_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_edge_property_type", (const class std::type_info & (easy3d::SurfaceMesh::*)(const std::string &) const) &easy3d::SurfaceMesh::get_edge_property_type, "get the type_info  of edge property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::SurfaceMesh::get_edge_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_face_property_type", (const class std::type_info & (easy3d::SurfaceMesh::*)(const std::string &) const) &easy3d::SurfaceMesh::get_face_property_type, "get the type_info  of face property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::SurfaceMesh::get_face_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("get_model_property_type", (const class std::type_info & (easy3d::SurfaceMesh::*)(const std::string &) const) &easy3d::SurfaceMesh::get_model_property_type, "get the type_info  of model property  returns an typeid(void)\n         if the property does not exist or if the type does not match. \n\nC++: easy3d::SurfaceMesh::get_model_property_type(const std::string &) const --> const class std::type_info &", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("vertex_properties", (class std::vector<std::string> (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::vertex_properties, "returns the names of all vertex properties\n\nC++: easy3d::SurfaceMesh::vertex_properties() const --> class std::vector<std::string>");
		cl.def("halfedge_properties", (class std::vector<std::string> (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::halfedge_properties, "returns the names of all halfedge properties\n\nC++: easy3d::SurfaceMesh::halfedge_properties() const --> class std::vector<std::string>");
		cl.def("edge_properties", (class std::vector<std::string> (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::edge_properties, "returns the names of all edge properties\n\nC++: easy3d::SurfaceMesh::edge_properties() const --> class std::vector<std::string>");
		cl.def("face_properties", (class std::vector<std::string> (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::face_properties, "returns the names of all face properties\n\nC++: easy3d::SurfaceMesh::face_properties() const --> class std::vector<std::string>");
		cl.def("model_properties", (class std::vector<std::string> (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::model_properties, "returns the names of all model properties\n\nC++: easy3d::SurfaceMesh::model_properties() const --> class std::vector<std::string>");
		cl.def("property_stats", (void (easy3d::SurfaceMesh::*)(std::ostream &) const) &easy3d::SurfaceMesh::property_stats, "prints the names of all properties to an output stream (e.g., std::cout).\n\nC++: easy3d::SurfaceMesh::property_stats(std::ostream &) const --> void", pybind11::arg("output"));
		cl.def("vertices_begin", (class easy3d::SurfaceMesh::VertexIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::vertices_begin, "returns start iterator for vertices\n\nC++: easy3d::SurfaceMesh::vertices_begin() const --> class easy3d::SurfaceMesh::VertexIterator");
		cl.def("vertices_end", (class easy3d::SurfaceMesh::VertexIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::vertices_end, "returns end iterator for vertices\n\nC++: easy3d::SurfaceMesh::vertices_end() const --> class easy3d::SurfaceMesh::VertexIterator");
		cl.def("vertices", (class easy3d::SurfaceMesh::VertexContainer (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::vertices, "returns vertex container for C++11 range-based for-loops\n\nC++: easy3d::SurfaceMesh::vertices() const --> class easy3d::SurfaceMesh::VertexContainer");
		cl.def("halfedges_begin", (class easy3d::SurfaceMesh::HalfedgeIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::halfedges_begin, "returns start iterator for halfedges\n\nC++: easy3d::SurfaceMesh::halfedges_begin() const --> class easy3d::SurfaceMesh::HalfedgeIterator");
		cl.def("halfedges_end", (class easy3d::SurfaceMesh::HalfedgeIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::halfedges_end, "returns end iterator for halfedges\n\nC++: easy3d::SurfaceMesh::halfedges_end() const --> class easy3d::SurfaceMesh::HalfedgeIterator");
		cl.def("halfedges", (class easy3d::SurfaceMesh::HalfedgeContainer (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::halfedges, "returns halfedge container for C++11 range-based for-loops\n\nC++: easy3d::SurfaceMesh::halfedges() const --> class easy3d::SurfaceMesh::HalfedgeContainer");
		cl.def("edges_begin", (class easy3d::SurfaceMesh::EdgeIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::edges_begin, "returns start iterator for edges\n\nC++: easy3d::SurfaceMesh::edges_begin() const --> class easy3d::SurfaceMesh::EdgeIterator");
		cl.def("edges_end", (class easy3d::SurfaceMesh::EdgeIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::edges_end, "returns end iterator for edges\n\nC++: easy3d::SurfaceMesh::edges_end() const --> class easy3d::SurfaceMesh::EdgeIterator");
		cl.def("edges", (class easy3d::SurfaceMesh::EdgeContainer (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::edges, "returns edge container for C++11 range-based for-loops\n\nC++: easy3d::SurfaceMesh::edges() const --> class easy3d::SurfaceMesh::EdgeContainer");
		cl.def("faces_begin", (class easy3d::SurfaceMesh::FaceIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::faces_begin, "returns start iterator for faces\n\nC++: easy3d::SurfaceMesh::faces_begin() const --> class easy3d::SurfaceMesh::FaceIterator");
		cl.def("faces_end", (class easy3d::SurfaceMesh::FaceIterator (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::faces_end, "returns end iterator for faces\n\nC++: easy3d::SurfaceMesh::faces_end() const --> class easy3d::SurfaceMesh::FaceIterator");
		cl.def("faces", (class easy3d::SurfaceMesh::FaceContainer (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::faces, "returns face container for C++11 range-based for-loops\n\nC++: easy3d::SurfaceMesh::faces() const --> class easy3d::SurfaceMesh::FaceContainer");
		cl.def("vertices", (class easy3d::SurfaceMesh::VertexAroundVertexCirculator (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::vertices, "returns circulator for vertices around vertex \n\nC++: easy3d::SurfaceMesh::vertices(struct easy3d::SurfaceMesh::Vertex) const --> class easy3d::SurfaceMesh::VertexAroundVertexCirculator", pybind11::arg("v"));
		cl.def("halfedges", (class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::halfedges, "returns circulator for outgoing halfedges around vertex \n\nC++: easy3d::SurfaceMesh::halfedges(struct easy3d::SurfaceMesh::Vertex) const --> class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator", pybind11::arg("v"));
		cl.def("faces", (class easy3d::SurfaceMesh::FaceAroundVertexCirculator (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::faces, "returns circulator for faces around vertex \n\nC++: easy3d::SurfaceMesh::faces(struct easy3d::SurfaceMesh::Vertex) const --> class easy3d::SurfaceMesh::FaceAroundVertexCirculator", pybind11::arg("v"));
		cl.def("vertices", (class easy3d::SurfaceMesh::VertexAroundFaceCirculator (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::vertices, "returns circulator for vertices of face \n\nC++: easy3d::SurfaceMesh::vertices(struct easy3d::SurfaceMesh::Face) const --> class easy3d::SurfaceMesh::VertexAroundFaceCirculator", pybind11::arg("f"));
		cl.def("halfedges", (class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::halfedges, "returns circulator for halfedges of face \n\nC++: easy3d::SurfaceMesh::halfedges(struct easy3d::SurfaceMesh::Face) const --> class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator", pybind11::arg("f"));
		cl.def("is_closed", (bool (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::is_closed, "returns whether the mesh closed (i.e., no boundary edges)\n\nC++: easy3d::SurfaceMesh::is_closed() const --> bool");
		cl.def("is_triangle_mesh", (bool (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::is_triangle_mesh, "returns whether the mesh a triangle mesh. this function simply tests\n each face, and therefore is not very efficient.\n \n\n triangulate(), triangulate(Face)\n\nC++: easy3d::SurfaceMesh::is_triangle_mesh() const --> bool");
		cl.def("is_quad_mesh", (bool (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::is_quad_mesh, "returns whether the mesh a quad mesh. this function simply tests\n each face, and therefore is not very efficient.\n\nC++: easy3d::SurfaceMesh::is_quad_mesh() const --> bool");
		cl.def("triangulate", (void (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::triangulate, "triangulate the entire mesh, by calling triangulate(Face) for each face.\n \n\n triangulate(Face)\n\nC++: easy3d::SurfaceMesh::triangulate() --> void");
		cl.def("triangulate", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face)) &easy3d::SurfaceMesh::triangulate, "triangulate the face \n \n\n triangulate()\n\nC++: easy3d::SurfaceMesh::triangulate(struct easy3d::SurfaceMesh::Face) --> void", pybind11::arg("f"));
		cl.def("reverse_orientation", (void (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::reverse_orientation, "Reverses the orientation of the entire mesh.\n \n\n This function reverses for each face the order of the vertices along the face boundary.\n As a consequence, the normal computed for each face using compute_face_normal() is also reversed.\n\nC++: easy3d::SurfaceMesh::reverse_orientation() --> void");
		cl.def("is_collapse_ok", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::is_collapse_ok, "returns whether collapsing the halfedge  is topologically legal.\n \n\n This function is only valid for triangle meshes.\n\nC++: easy3d::SurfaceMesh::is_collapse_ok(struct easy3d::SurfaceMesh::Halfedge) const --> bool", pybind11::arg("h"));
		cl.def("collapse", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::collapse, "Collapse the halfedge  by moving its start vertex into its target\n         vertex. For non-boundary halfedges this function removes one vertex, three\n         edges, and two faces. For boundary halfedges it removes one vertex, two\n         edges and one face.\n         \n\n This function is only valid for triangle meshes.\n         \n\n Halfedge collapses might lead to invalid faces. Call\n         is_collapse_ok(Halfedge) to be sure the collapse is legal.\n         \n\n The removed items are only marked as deleted. You have\n         to call collect_garbage() to finally remove them.\n\nC++: easy3d::SurfaceMesh::collapse(struct easy3d::SurfaceMesh::Halfedge) --> void", pybind11::arg("h"));
		cl.def("split", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face, const class easy3d::Vec<3, float> &)) &easy3d::SurfaceMesh::split, "Split the face  by first adding point  to the mesh and then\n         inserting edges between  and the vertices of  For a triangle\n         this is a standard one-to-three split.\n         \n\n split(Face, Vertex)\n\nC++: easy3d::SurfaceMesh::split(struct easy3d::SurfaceMesh::Face, const class easy3d::Vec<3, float> &) --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("f"), pybind11::arg("p"));
		cl.def("split", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::split, "Split the face  by inserting edges between  and the vertices\n         of  For a triangle this is a standard one-to-three split.\n         \n\n split(Face, const vec3&)\n\nC++: easy3d::SurfaceMesh::split(struct easy3d::SurfaceMesh::Face, struct easy3d::SurfaceMesh::Vertex) --> void", pybind11::arg("f"), pybind11::arg("v"));
		cl.def("split", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, const class easy3d::Vec<3, float> &)) &easy3d::SurfaceMesh::split, "Split the edge  by first adding point  to the mesh and then\n         connecting it to the two vertices of the adjacent triangles that are\n         opposite to edge  Returns the halfedge pointing to  that is\n         created by splitting the existing edge \n\n         \n This function is only valid for triangle meshes.\n         \n\n split(Edge, Vertex)\n\nC++: easy3d::SurfaceMesh::split(struct easy3d::SurfaceMesh::Edge, const class easy3d::Vec<3, float> &) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("e"), pybind11::arg("p"));
		cl.def("split", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::split, "Split the edge  by connecting vertex  it to the two\n         vertices of the adjacent triangles that are opposite to edge \n Returns the halfedge pointing to  that is created by splitting\n         the existing edge \n\n         \n This function is only valid for triangle meshes.\n         \n\n split(Edge, vec3)\n\nC++: easy3d::SurfaceMesh::split(struct easy3d::SurfaceMesh::Edge, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("e"), pybind11::arg("v"));
		cl.def("insert_vertex", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, const class easy3d::Vec<3, float> &)) &easy3d::SurfaceMesh::insert_vertex, "Subdivide the edge  = (v0,v1) by splitting it into the two edge\n         (v0,p) and (p,v1). Note that this function does not introduce any\n         other edge or faces. It simply splits the edge. Returns halfedge that\n         points to \n         \n\n insert_vertex(Edge, Vertex)\n         \n\n insert_vertex(Halfedge, Vertex)\n         \n\n\n         \n\nC++: easy3d::SurfaceMesh::insert_vertex(struct easy3d::SurfaceMesh::Edge, const class easy3d::Vec<3, float> &) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("e"), pybind11::arg("p"));
		cl.def("insert_vertex", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::insert_vertex, "Subdivide the edge  = (v0,v1) by splitting it into the two edge\n         (v0,v) and (v,v1). Note that this function does not introduce any other\n         edge or faces. It simply splits the edge. Returns halfedge that points to \n         \n\n insert_vertex(Edge, vec3)\n         \n\n insert_vertex(Halfedge, Vertex)\n         \n\n\n         \n\nC++: easy3d::SurfaceMesh::insert_vertex(struct easy3d::SurfaceMesh::Edge, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("e"), pybind11::arg("v"));
		cl.def("insert_vertex", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::insert_vertex, "Subdivide the edge  = (v0,v1) by splitting it into the two edge\n         (v0,v) and (v,v1). Note that this function does not introduce any other\n         edge or faces. It simply splits the edge. Returns halfedge that points to \n         \n\n insert_vertex(Edge, vec3)\n         \n\n insert_vertex(Edge, Vertex)\n         \n\n\n         \n\nC++: easy3d::SurfaceMesh::insert_vertex(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h"), pybind11::arg("v"));
		cl.def("join_edges", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::join_edges, "Merges the two incident edges of a 2-degree vertex. This is the reverse operation of insert_vertex().\n \n\n valence(v) == 2.\n \n\n insert_vertex(Edge, vec3)\n \n\n insert_vertex(Edge, Vertex)\n \n\n insert_vertex(Halfedge, Vertex)\n\nC++: easy3d::SurfaceMesh::join_edges(struct easy3d::SurfaceMesh::Vertex) --> bool", pybind11::arg("v"));
		cl.def("can_join_edges", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::can_join_edges, "Check whether the two incident edges of a vertex can be joined. It only allows for vertices of valence two.\n \n\n join_edges(Vertex)\n\nC++: easy3d::SurfaceMesh::can_join_edges(struct easy3d::SurfaceMesh::Vertex) const --> bool", pybind11::arg("v"));
		cl.def("insert_edge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::insert_edge, "insert edge between the to-vertices v0 of h0 and v1 of h1.\n returns the new halfedge from v0 to v1.\n \n\n h0 and h1 have to belong to the same face\n\nC++: easy3d::SurfaceMesh::insert_edge(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("h0"), pybind11::arg("h1"));
		cl.def("is_flip_ok", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge) const) &easy3d::SurfaceMesh::is_flip_ok, "Check whether flipping edge  is topologically allowed.\n         \n\n This function is only valid for triangle meshes.\n         \n\n flip(Edge)\n\nC++: easy3d::SurfaceMesh::is_flip_ok(struct easy3d::SurfaceMesh::Edge) const --> bool", pybind11::arg("e"));
		cl.def("flip", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge)) &easy3d::SurfaceMesh::flip, "Flip edge  Remove edge  and add an edge between the two vertices opposite to edge  of the two\n incident triangles.\n \n\n This function is only valid for triangle meshes. \n is_flip_ok(Edge)\n\nC++: easy3d::SurfaceMesh::flip(struct easy3d::SurfaceMesh::Edge) --> void", pybind11::arg("e"));
		cl.def("is_stitch_ok", (bool (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::is_stitch_ok, "Check whether stitching two halfedges  an  is topologically allowed. Two halfedges can be stitched\n if they are both on on the border and point in reverse directions.\n\nC++: easy3d::SurfaceMesh::is_stitch_ok(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge) --> bool", pybind11::arg("h0"), pybind11::arg("h1"));
		cl.def("stitch", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::stitch, "Stitch two halfedges h0 and h1. Precondition: h0 and h1 are both on the border and point in reversed\n      directions.\n \n\n Stitching two halfedges changes the topology and geometry significantly, thus it may result in a\n      non-manifold mesh, client code must check if this operation can be executed. \n\n is_stitch_ok().\n\nC++: easy3d::SurfaceMesh::stitch(struct easy3d::SurfaceMesh::Halfedge, struct easy3d::SurfaceMesh::Halfedge) --> void", pybind11::arg("h0"), pybind11::arg("h1"));
		cl.def("valence", (unsigned int (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::valence, "returns the valence (number of incident edges or neighboring vertices)\n         of vertex  \n\nC++: easy3d::SurfaceMesh::valence(struct easy3d::SurfaceMesh::Vertex) const --> unsigned int", pybind11::arg("v"));
		cl.def("valence", (unsigned int (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::valence, "returns the valence of face  (its number of vertices)\n\nC++: easy3d::SurfaceMesh::valence(struct easy3d::SurfaceMesh::Face) const --> unsigned int", pybind11::arg("f"));
		cl.def("find_halfedge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::find_halfedge, "find the halfedge from start to end\n\nC++: easy3d::SurfaceMesh::find_halfedge(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) const --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("start"), pybind11::arg("end"));
		cl.def("find_edge", (struct easy3d::SurfaceMesh::Edge (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::find_edge, "find the edge (a, b)\n\nC++: easy3d::SurfaceMesh::find_edge(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) const --> struct easy3d::SurfaceMesh::Edge", pybind11::arg("a"), pybind11::arg("b"));
		cl.def("delete_vertex", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::delete_vertex, "deletes the vertex  from the mesh. Its incident edges and faces will also be deleted.\n \n\n This function only marks the vertex and its incident edges and faces as deleted, and you have\n     to call collect_garbage() to finally remove them.\n\nC++: easy3d::SurfaceMesh::delete_vertex(struct easy3d::SurfaceMesh::Vertex) --> void", pybind11::arg("v"));
		cl.def("delete_edge", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge)) &easy3d::SurfaceMesh::delete_edge, "deletes the edge  from the mesh. Its incident faces will also be deleted.\n \n\n This function only marks the edge and its incident faces as deleted, and you have to call\n     collect_garbage() to finally remove them.\n\nC++: easy3d::SurfaceMesh::delete_edge(struct easy3d::SurfaceMesh::Edge) --> void", pybind11::arg("e"));
		cl.def("delete_face", (void (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face)) &easy3d::SurfaceMesh::delete_face, "deletes the face  from the mesh. Its incident edges (if on boundary) will also be deleted.\n \n\n This function only marks the face and its incident edges (if on boundary) as deleted, and\n     you have to call collect_garbage() to finally remove them.\n\nC++: easy3d::SurfaceMesh::delete_face(struct easy3d::SurfaceMesh::Face) --> void", pybind11::arg("f"));
		cl.def("position", (class easy3d::Vec<3, float> & (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::position, "position of a vertex\n\nC++: easy3d::SurfaceMesh::position(struct easy3d::SurfaceMesh::Vertex) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("points", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::points, "vector of vertex positions\n\nC++: easy3d::SurfaceMesh::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
		cl.def("points", (const class std::vector<class easy3d::Vec<3, float> > & (easy3d::SurfaceMesh::*)() const) &easy3d::SurfaceMesh::points, "vector of vertex positions\n\nC++: easy3d::SurfaceMesh::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
		cl.def("update_face_normals", (void (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::update_face_normals, "compute face normals by calling compute_face_normal(Face) for each face.\n\nC++: easy3d::SurfaceMesh::update_face_normals() --> void");
		cl.def("compute_face_normal", (class easy3d::Vec<3, float> (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Face) const) &easy3d::SurfaceMesh::compute_face_normal, "compute normal vector of face  This method is robust for concave and general polygonal faces.\n\nC++: easy3d::SurfaceMesh::compute_face_normal(struct easy3d::SurfaceMesh::Face) const --> class easy3d::Vec<3, float>", pybind11::arg("f"));
		cl.def("update_vertex_normals", (void (easy3d::SurfaceMesh::*)()) &easy3d::SurfaceMesh::update_vertex_normals, "compute vertex normals by calling compute_vertex_normal(Vertex) for each vertex.\n\nC++: easy3d::SurfaceMesh::update_vertex_normals() --> void");
		cl.def("compute_vertex_normal", (class easy3d::Vec<3, float> (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMesh::compute_vertex_normal, "compute normal vector of vertex  This is the angle-weighted average of incident face normals.\n \n\n The per-face normals much have been computed.\n\nC++: easy3d::SurfaceMesh::compute_vertex_normal(struct easy3d::SurfaceMesh::Vertex) const --> class easy3d::Vec<3, float>", pybind11::arg("v"));
		cl.def("edge_length", (float (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Edge) const) &easy3d::SurfaceMesh::edge_length, "compute the length of edge \n\nC++: easy3d::SurfaceMesh::edge_length(struct easy3d::SurfaceMesh::Edge) const --> float", pybind11::arg("e"));
		cl.def("edge_length", (float (easy3d::SurfaceMesh::*)(struct easy3d::SurfaceMesh::Halfedge) const) &easy3d::SurfaceMesh::edge_length, "compute the length of an edge denoted by one of its halfedge \n\nC++: easy3d::SurfaceMesh::edge_length(struct easy3d::SurfaceMesh::Halfedge) const --> float", pybind11::arg("h"));

		{ // easy3d::SurfaceMesh::BaseHandle file:easy3d/core/surface_mesh.h line:59
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::BaseHandle, std::shared_ptr<easy3d::SurfaceMesh::BaseHandle>> cl(enclosing_class, "BaseHandle", "Base class for all topology types (internally it is basically an index)\n \n\n Vertex, Halfedge, Edge, Face");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::BaseHandle(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::BaseHandle const &o){ return new easy3d::SurfaceMesh::BaseHandle(o); } ) );
			cl.def("idx", (int (easy3d::SurfaceMesh::BaseHandle::*)() const) &easy3d::SurfaceMesh::BaseHandle::idx, "Get the underlying index of this handle\n\nC++: easy3d::SurfaceMesh::BaseHandle::idx() const --> int");
			cl.def("reset", (void (easy3d::SurfaceMesh::BaseHandle::*)()) &easy3d::SurfaceMesh::BaseHandle::reset, "reset handle to be invalid (index=-1)\n\nC++: easy3d::SurfaceMesh::BaseHandle::reset() --> void");
			cl.def("is_valid", (bool (easy3d::SurfaceMesh::BaseHandle::*)() const) &easy3d::SurfaceMesh::BaseHandle::is_valid, "return whether the handle is valid, i.e., the index is not equal to -1.\n\nC++: easy3d::SurfaceMesh::BaseHandle::is_valid() const --> bool");
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::BaseHandle::*)(const class easy3d::SurfaceMesh::BaseHandle &) const) &easy3d::SurfaceMesh::BaseHandle::operator==, "are two handles equal?\n\nC++: easy3d::SurfaceMesh::BaseHandle::operator==(const class easy3d::SurfaceMesh::BaseHandle &) const --> bool", pybind11::arg("_rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::BaseHandle::*)(const class easy3d::SurfaceMesh::BaseHandle &) const) &easy3d::SurfaceMesh::BaseHandle::operator!=, "are two handles different?\n\nC++: easy3d::SurfaceMesh::BaseHandle::operator!=(const class easy3d::SurfaceMesh::BaseHandle &) const --> bool", pybind11::arg("_rhs"));
			cl.def("assign", (class easy3d::SurfaceMesh::BaseHandle & (easy3d::SurfaceMesh::BaseHandle::*)(const class easy3d::SurfaceMesh::BaseHandle &)) &easy3d::SurfaceMesh::BaseHandle::operator=, "C++: easy3d::SurfaceMesh::BaseHandle::operator=(const class easy3d::SurfaceMesh::BaseHandle &) --> class easy3d::SurfaceMesh::BaseHandle &", pybind11::return_value_policy::automatic, pybind11::arg(""));

			{ // easy3d::SurfaceMesh::BaseHandle::Hash file:easy3d/core/surface_mesh.h line:91
				auto & enclosing_class = cl;
				pybind11::class_<easy3d::SurfaceMesh::BaseHandle::Hash, std::shared_ptr<easy3d::SurfaceMesh::BaseHandle::Hash>> cl(enclosing_class, "Hash", "helper structure to be able to use std::unordered_map");
				cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::BaseHandle::Hash(); } ) );
				cl.def("__call__", (std::size_t (easy3d::SurfaceMesh::BaseHandle::Hash::*)(const class easy3d::SurfaceMesh::BaseHandle &) const) &easy3d::SurfaceMesh::BaseHandle::Hash::operator(), "C++: easy3d::SurfaceMesh::BaseHandle::Hash::operator()(const class easy3d::SurfaceMesh::BaseHandle &) const --> std::size_t", pybind11::arg("h"));
			}

		}

		{ // easy3d::SurfaceMesh::Vertex file:easy3d/core/surface_mesh.h line:103
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::Vertex, std::shared_ptr<easy3d::SurfaceMesh::Vertex>, easy3d::SurfaceMesh::BaseHandle> cl(enclosing_class, "Vertex", "this type represents a vertex (internally it is basically an index)\n  \n\n Halfedge, Edge, Face");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::Vertex(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::Vertex const &o){ return new easy3d::SurfaceMesh::Vertex(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::SurfaceMesh::Vertex::*)(std::ostream &) const) &easy3d::SurfaceMesh::Vertex::operator<<, "C++: easy3d::SurfaceMesh::Vertex::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::SurfaceMesh::Vertex & (easy3d::SurfaceMesh::Vertex::*)(const struct easy3d::SurfaceMesh::Vertex &)) &easy3d::SurfaceMesh::Vertex::operator=, "C++: easy3d::SurfaceMesh::Vertex::operator=(const struct easy3d::SurfaceMesh::Vertex &) --> struct easy3d::SurfaceMesh::Vertex &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::Halfedge file:easy3d/core/surface_mesh.h line:113
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::Halfedge, std::shared_ptr<easy3d::SurfaceMesh::Halfedge>, easy3d::SurfaceMesh::BaseHandle> cl(enclosing_class, "Halfedge", "this type represents a halfedge (internally it is basically an index)\n \n\n Vertex, Edge, Face");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::Halfedge(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::Halfedge const &o){ return new easy3d::SurfaceMesh::Halfedge(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::SurfaceMesh::Halfedge::*)(std::ostream &) const) &easy3d::SurfaceMesh::Halfedge::operator<<, "C++: easy3d::SurfaceMesh::Halfedge::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::SurfaceMesh::Halfedge & (easy3d::SurfaceMesh::Halfedge::*)(const struct easy3d::SurfaceMesh::Halfedge &)) &easy3d::SurfaceMesh::Halfedge::operator=, "C++: easy3d::SurfaceMesh::Halfedge::operator=(const struct easy3d::SurfaceMesh::Halfedge &) --> struct easy3d::SurfaceMesh::Halfedge &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::Edge file:easy3d/core/surface_mesh.h line:123
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::Edge, std::shared_ptr<easy3d::SurfaceMesh::Edge>, easy3d::SurfaceMesh::BaseHandle> cl(enclosing_class, "Edge", "this type represents an edge (internally it is basically an index)\n \n\n Vertex, Halfedge, Face");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::Edge(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::Edge const &o){ return new easy3d::SurfaceMesh::Edge(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::SurfaceMesh::Edge::*)(std::ostream &) const) &easy3d::SurfaceMesh::Edge::operator<<, "C++: easy3d::SurfaceMesh::Edge::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
		}

		{ // easy3d::SurfaceMesh::Face file:easy3d/core/surface_mesh.h line:133
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::Face, std::shared_ptr<easy3d::SurfaceMesh::Face>, easy3d::SurfaceMesh::BaseHandle> cl(enclosing_class, "Face", "this type represents a face (internally it is basically an index)\n \n\n Vertex, Halfedge, Edge");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::Face(); } ), "doc" );
			cl.def( pybind11::init<int>(), pybind11::arg("_idx") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::Face const &o){ return new easy3d::SurfaceMesh::Face(o); } ) );
			cl.def("__lshift__", (std::ostream & (easy3d::SurfaceMesh::Face::*)(std::ostream &) const) &easy3d::SurfaceMesh::Face::operator<<, "C++: easy3d::SurfaceMesh::Face::operator<<(std::ostream &) const --> std::ostream &", pybind11::return_value_policy::automatic, pybind11::arg("os"));
			cl.def("assign", (struct easy3d::SurfaceMesh::Face & (easy3d::SurfaceMesh::Face::*)(const struct easy3d::SurfaceMesh::Face &)) &easy3d::SurfaceMesh::Face::operator=, "C++: easy3d::SurfaceMesh::Face::operator=(const struct easy3d::SurfaceMesh::Face &) --> struct easy3d::SurfaceMesh::Face &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::VertexConnectivity file:easy3d/core/surface_mesh.h line:147
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexConnectivity, std::shared_ptr<easy3d::SurfaceMesh::VertexConnectivity>> cl(enclosing_class, "VertexConnectivity", "This type stores the vertex connectivity\n \n\n HalfedgeConnectivity, FaceConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexConnectivity const &o){ return new easy3d::SurfaceMesh::VertexConnectivity(o); } ) );
			cl.def_readwrite("halfedge_", &easy3d::SurfaceMesh::VertexConnectivity::halfedge_);
			cl.def("assign", (struct easy3d::SurfaceMesh::VertexConnectivity & (easy3d::SurfaceMesh::VertexConnectivity::*)(const struct easy3d::SurfaceMesh::VertexConnectivity &)) &easy3d::SurfaceMesh::VertexConnectivity::operator=, "C++: easy3d::SurfaceMesh::VertexConnectivity::operator=(const struct easy3d::SurfaceMesh::VertexConnectivity &) --> struct easy3d::SurfaceMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::HalfedgeConnectivity file:easy3d/core/surface_mesh.h line:156
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::HalfedgeConnectivity, std::shared_ptr<easy3d::SurfaceMesh::HalfedgeConnectivity>> cl(enclosing_class, "HalfedgeConnectivity", "This type stores the halfedge connectivity\n \n\n VertexConnectivity, FaceConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::HalfedgeConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::HalfedgeConnectivity const &o){ return new easy3d::SurfaceMesh::HalfedgeConnectivity(o); } ) );
			cl.def_readwrite("face_", &easy3d::SurfaceMesh::HalfedgeConnectivity::face_);
			cl.def_readwrite("vertex_", &easy3d::SurfaceMesh::HalfedgeConnectivity::vertex_);
			cl.def_readwrite("next_", &easy3d::SurfaceMesh::HalfedgeConnectivity::next_);
			cl.def_readwrite("prev_", &easy3d::SurfaceMesh::HalfedgeConnectivity::prev_);
			cl.def("assign", (struct easy3d::SurfaceMesh::HalfedgeConnectivity & (easy3d::SurfaceMesh::HalfedgeConnectivity::*)(const struct easy3d::SurfaceMesh::HalfedgeConnectivity &)) &easy3d::SurfaceMesh::HalfedgeConnectivity::operator=, "C++: easy3d::SurfaceMesh::HalfedgeConnectivity::operator=(const struct easy3d::SurfaceMesh::HalfedgeConnectivity &) --> struct easy3d::SurfaceMesh::HalfedgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::FaceConnectivity file:easy3d/core/surface_mesh.h line:171
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceConnectivity, std::shared_ptr<easy3d::SurfaceMesh::FaceConnectivity>> cl(enclosing_class, "FaceConnectivity", "This type stores the face connectivity\n \n\n VertexConnectivity, HalfedgeConnectivity");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::FaceConnectivity(); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceConnectivity const &o){ return new easy3d::SurfaceMesh::FaceConnectivity(o); } ) );
			cl.def_readwrite("halfedge_", &easy3d::SurfaceMesh::FaceConnectivity::halfedge_);
			cl.def("assign", (struct easy3d::SurfaceMesh::FaceConnectivity & (easy3d::SurfaceMesh::FaceConnectivity::*)(const struct easy3d::SurfaceMesh::FaceConnectivity &)) &easy3d::SurfaceMesh::FaceConnectivity::operator=, "C++: easy3d::SurfaceMesh::FaceConnectivity::operator=(const struct easy3d::SurfaceMesh::FaceConnectivity &) --> struct easy3d::SurfaceMesh::FaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>, std::shared_ptr<easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>>, PyCallBack_easy3d_Property_easy3d_SurfaceMesh_VertexConnectivity_t> cl(m, "Property_easy3d_SurfaceMesh_VertexConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_VertexConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_SurfaceMesh_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_VertexConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity> const &o){ return new easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::SurfaceMesh::VertexConnectivity & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::SurfaceMesh::VertexConnectivity * (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::data() const --> const struct easy3d::SurfaceMesh::VertexConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> &)) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }
        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>, std::shared_ptr<easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>>, PyCallBack_easy3d_Property_easy3d_SurfaceMesh_HalfedgeConnectivity_t> cl(m, "Property_easy3d_SurfaceMesh_HalfedgeConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_HalfedgeConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_SurfaceMesh_HalfedgeConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_HalfedgeConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity> const &o){ return new easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::SurfaceMesh::HalfedgeConnectivity & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::HalfedgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::SurfaceMesh::HalfedgeConnectivity * (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::data() const --> const struct easy3d::SurfaceMesh::HalfedgeConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &)) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }
        { // easy3d::Property file:easy3d/core/property.h line:252
            pybind11::class_<easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>, std::shared_ptr<easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>>, PyCallBack_easy3d_Property_easy3d_SurfaceMesh_FaceConnectivity_t> cl(m, "Property_easy3d_SurfaceMesh_FaceConnectivity_t", "");
            cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_FaceConnectivity_t(); } ), "doc");
            cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> *>(), pybind11::arg("p") );

            cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_SurfaceMesh_FaceConnectivity_t const &o){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_FaceConnectivity_t(o); } ) );
            cl.def( pybind11::init( [](easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity> const &o){ return new easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>(o); } ) );
            cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::reset() --> void");
            cl.def("__getitem__", (struct easy3d::SurfaceMesh::FaceConnectivity & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::FaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
            cl.def("data", (const struct easy3d::SurfaceMesh::FaceConnectivity * (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::data() const --> const struct easy3d::SurfaceMesh::FaceConnectivity *", pybind11::return_value_policy::automatic);
            cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic);
            cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
            cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
            cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> &)) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
        }

		{ // easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>, std::shared_ptr<easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>>, PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_SurfaceMesh_VertexConnectivity_t, easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>> cl(enclosing_class, "VertexProperty_easy3d_SurfaceMesh_VertexConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_SurfaceMesh_VertexConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_SurfaceMesh_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_SurfaceMesh_VertexConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity> const &o){ return new easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::SurfaceMesh::VertexConnectivity & (easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::*)(struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::operator[], "C++: easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::operator[](struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::SurfaceMesh::VertexProperty<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::*)(const class easy3d::SurfaceMesh::VertexProperty<struct easy3d::SurfaceMesh::VertexConnectivity> &)) &easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::operator=, "C++: easy3d::SurfaceMesh::VertexProperty<easy3d::SurfaceMesh::VertexConnectivity>::operator=(const class easy3d::SurfaceMesh::VertexProperty<struct easy3d::SurfaceMesh::VertexConnectivity> &) --> class easy3d::SurfaceMesh::VertexProperty<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::SurfaceMesh::VertexConnectivity & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::SurfaceMesh::VertexConnectivity * (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::data() const --> const struct easy3d::SurfaceMesh::VertexConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> &)) &easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::VertexConnectivity>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexProperty<bool>, std::shared_ptr<easy3d::SurfaceMesh::VertexProperty<bool>>, easy3d::Property<bool>> cl(enclosing_class, "VertexProperty_bool_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexProperty<bool>(); } ) );
			cl.def( pybind11::init<class easy3d::Property<bool>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexProperty<bool> const &o){ return new easy3d::SurfaceMesh::VertexProperty<bool>(o); } ) );
			cl.def("assign", (class easy3d::SurfaceMesh::VertexProperty<bool> & (easy3d::SurfaceMesh::VertexProperty<bool>::*)(const class easy3d::SurfaceMesh::VertexProperty<bool> &)) &easy3d::SurfaceMesh::VertexProperty<bool>::operator=, "C++: easy3d::SurfaceMesh::VertexProperty<bool>::operator=(const class easy3d::SurfaceMesh::VertexProperty<bool> &) --> class easy3d::SurfaceMesh::VertexProperty<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::reset, "C++: easy3d::Property<bool>::reset() --> void");
			cl.def("data", (const bool * (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::data, "C++: easy3d::Property<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::vector, "C++: easy3d::Property<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::array, "C++: easy3d::Property<bool>::array() --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::name, "C++: easy3d::Property<bool>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<bool>::*)(const std::string &)) &easy3d::Property<bool>::set_name, "C++: easy3d::Property<bool>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<bool> & (easy3d::Property<bool>::*)(const class easy3d::Property<bool> &)) &easy3d::Property<bool>::operator=, "C++: easy3d::Property<bool>::operator=(const class easy3d::Property<bool> &) --> class easy3d::Property<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>, std::shared_ptr<easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>>, PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_Vec_3_float_t, easy3d::Property<easy3d::Vec<3, float>>> cl(enclosing_class, "VertexProperty_easy3d_Vec_3_float_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_Vec_3_float_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<class easy3d::Vec<3, float> >>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_Vec_3_float_t const &o){ return new PyCallBack_easy3d_SurfaceMesh_VertexProperty_easy3d_Vec_3_float_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>> const &o){ return new easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>(o); } ) );
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::*)(struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::operator[], "C++: easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::operator[](struct easy3d::SurfaceMesh::Vertex) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::SurfaceMesh::VertexProperty<class easy3d::Vec<3, float> > & (easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::*)(const class easy3d::SurfaceMesh::VertexProperty<class easy3d::Vec<3, float> > &)) &easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::operator=, "C++: easy3d::SurfaceMesh::VertexProperty<easy3d::Vec<3, float>>::operator=(const class easy3d::SurfaceMesh::VertexProperty<class easy3d::Vec<3, float> > &) --> class easy3d::SurfaceMesh::VertexProperty<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::reset, "C++: easy3d::Property<easy3d::Vec<3, float>>::reset() --> void");
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::Property<easy3d::Vec<3, float>>::*)(size_t)) &easy3d::Property<easy3d::Vec<3, float>>::operator[], "C++: easy3d::Property<easy3d::Vec<3, float>>::operator[](size_t) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const class easy3d::Vec<3, float> * (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::data, "C++: easy3d::Property<easy3d::Vec<3, float>>::data() const --> const class easy3d::Vec<3, float> *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::vector, "C++: easy3d::Property<easy3d::Vec<3, float>>::vector() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::array, "C++: easy3d::Property<easy3d::Vec<3, float>>::array() --> class easy3d::PropertyArray<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::name, "C++: easy3d::Property<easy3d::Vec<3, float>>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::Vec<3, float>>::*)(const std::string &)) &easy3d::Property<easy3d::Vec<3, float>>::set_name, "C++: easy3d::Property<easy3d::Vec<3, float>>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)(const class easy3d::Property<class easy3d::Vec<3, float> > &)) &easy3d::Property<easy3d::Vec<3, float>>::operator=, "C++: easy3d::Property<easy3d::Vec<3, float>>::operator=(const class easy3d::Property<class easy3d::Vec<3, float> > &) --> class easy3d::Property<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::VertexProperty file:easy3d/core/surface_mesh.h line:184
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexProperty<float>, std::shared_ptr<easy3d::SurfaceMesh::VertexProperty<float>>, PyCallBack_easy3d_SurfaceMesh_VertexProperty_float_t, easy3d::Property<float>> cl(enclosing_class, "VertexProperty_float_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexProperty<float>(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh_VertexProperty_float_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<float>>(), pybind11::arg("p") );

			cl.def("__getitem__", (float & (easy3d::SurfaceMesh::VertexProperty<float>::*)(struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMesh::VertexProperty<float>::operator[], "C++: easy3d::SurfaceMesh::VertexProperty<float>::operator[](struct easy3d::SurfaceMesh::Vertex) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
			cl.def("assign", (class easy3d::SurfaceMesh::VertexProperty<float> & (easy3d::SurfaceMesh::VertexProperty<float>::*)(const class easy3d::SurfaceMesh::VertexProperty<float> &)) &easy3d::SurfaceMesh::VertexProperty<float>::operator=, "C++: easy3d::SurfaceMesh::VertexProperty<float>::operator=(const class easy3d::SurfaceMesh::VertexProperty<float> &) --> class easy3d::SurfaceMesh::VertexProperty<float> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<float>::*)()) &easy3d::Property<float>::reset, "C++: easy3d::Property<float>::reset() --> void");
			cl.def("__getitem__", (float & (easy3d::Property<float>::*)(size_t)) &easy3d::Property<float>::operator[], "C++: easy3d::Property<float>::operator[](size_t) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const float * (easy3d::Property<float>::*)() const) &easy3d::Property<float>::data, "C++: easy3d::Property<float>::data() const --> const float *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<float> & (easy3d::Property<float>::*)()) &easy3d::Property<float>::vector, "C++: easy3d::Property<float>::vector() --> class std::vector<float> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<float> & (easy3d::Property<float>::*)()) &easy3d::Property<float>::array, "C++: easy3d::Property<float>::array() --> class easy3d::PropertyArray<float> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<float>::*)() const) &easy3d::Property<float>::name, "C++: easy3d::Property<float>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<float>::*)(const std::string &)) &easy3d::Property<float>::set_name, "C++: easy3d::Property<float>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<float> & (easy3d::Property<float>::*)(const class easy3d::Property<float> &)) &easy3d::Property<float>::operator=, "C++: easy3d::Property<float>::operator=(const class easy3d::Property<float> &) --> class easy3d::Property<float> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::HalfedgeProperty file:easy3d/core/surface_mesh.h line:208
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>, std::shared_ptr<easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>>, PyCallBack_easy3d_SurfaceMesh_HalfedgeProperty_easy3d_SurfaceMesh_HalfedgeConnectivity_t, easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>> cl(enclosing_class, "HalfedgeProperty_easy3d_SurfaceMesh_HalfedgeConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh_HalfedgeProperty_easy3d_SurfaceMesh_HalfedgeConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_SurfaceMesh_HalfedgeProperty_easy3d_SurfaceMesh_HalfedgeConnectivity_t const &o){ return new PyCallBack_easy3d_SurfaceMesh_HalfedgeProperty_easy3d_SurfaceMesh_HalfedgeConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity> const &o){ return new easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::SurfaceMesh::HalfedgeConnectivity & (easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[], "C++: easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[](struct easy3d::SurfaceMesh::Halfedge) --> struct easy3d::SurfaceMesh::HalfedgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("h"));
			cl.def("assign", (class easy3d::SurfaceMesh::HalfedgeProperty<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const class easy3d::SurfaceMesh::HalfedgeProperty<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &)) &easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=, "C++: easy3d::SurfaceMesh::HalfedgeProperty<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=(const class easy3d::SurfaceMesh::HalfedgeProperty<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &) --> class easy3d::SurfaceMesh::HalfedgeProperty<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::SurfaceMesh::HalfedgeConnectivity & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::HalfedgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::SurfaceMesh::HalfedgeConnectivity * (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::data() const --> const struct easy3d::SurfaceMesh::HalfedgeConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &)) &easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::EdgeProperty file:easy3d/core/surface_mesh.h line:232
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::EdgeProperty<bool>, std::shared_ptr<easy3d::SurfaceMesh::EdgeProperty<bool>>, easy3d::Property<bool>> cl(enclosing_class, "EdgeProperty_bool_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::EdgeProperty<bool>(); } ) );
			cl.def( pybind11::init<class easy3d::Property<bool>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::EdgeProperty<bool> const &o){ return new easy3d::SurfaceMesh::EdgeProperty<bool>(o); } ) );
			cl.def("assign", (class easy3d::SurfaceMesh::EdgeProperty<bool> & (easy3d::SurfaceMesh::EdgeProperty<bool>::*)(const class easy3d::SurfaceMesh::EdgeProperty<bool> &)) &easy3d::SurfaceMesh::EdgeProperty<bool>::operator=, "C++: easy3d::SurfaceMesh::EdgeProperty<bool>::operator=(const class easy3d::SurfaceMesh::EdgeProperty<bool> &) --> class easy3d::SurfaceMesh::EdgeProperty<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::reset, "C++: easy3d::Property<bool>::reset() --> void");
			cl.def("data", (const bool * (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::data, "C++: easy3d::Property<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::vector, "C++: easy3d::Property<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::array, "C++: easy3d::Property<bool>::array() --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::name, "C++: easy3d::Property<bool>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<bool>::*)(const std::string &)) &easy3d::Property<bool>::set_name, "C++: easy3d::Property<bool>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<bool> & (easy3d::Property<bool>::*)(const class easy3d::Property<bool> &)) &easy3d::Property<bool>::operator=, "C++: easy3d::Property<bool>::operator=(const class easy3d::Property<bool> &) --> class easy3d::Property<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::FaceProperty file:easy3d/core/surface_mesh.h line:256
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>, std::shared_ptr<easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>>, PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_SurfaceMesh_FaceConnectivity_t, easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>> cl(enclosing_class, "FaceProperty_easy3d_SurfaceMesh_FaceConnectivity_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_SurfaceMesh_FaceConnectivity_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_SurfaceMesh_FaceConnectivity_t const &o){ return new PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_SurfaceMesh_FaceConnectivity_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity> const &o){ return new easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>(o); } ) );
			cl.def("__getitem__", (struct easy3d::SurfaceMesh::FaceConnectivity & (easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::*)(struct easy3d::SurfaceMesh::Face)) &easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::operator[], "C++: easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::operator[](struct easy3d::SurfaceMesh::Face) --> struct easy3d::SurfaceMesh::FaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("f"));
			cl.def("assign", (class easy3d::SurfaceMesh::FaceProperty<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::*)(const class easy3d::SurfaceMesh::FaceProperty<struct easy3d::SurfaceMesh::FaceConnectivity> &)) &easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::operator=, "C++: easy3d::SurfaceMesh::FaceProperty<easy3d::SurfaceMesh::FaceConnectivity>::operator=(const class easy3d::SurfaceMesh::FaceProperty<struct easy3d::SurfaceMesh::FaceConnectivity> &) --> class easy3d::SurfaceMesh::FaceProperty<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::reset() --> void");
			cl.def("__getitem__", (struct easy3d::SurfaceMesh::FaceConnectivity & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::FaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const struct easy3d::SurfaceMesh::FaceConnectivity * (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::data() const --> const struct easy3d::SurfaceMesh::FaceConnectivity *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> &)) &easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::FaceConnectivity>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::FaceProperty file:easy3d/core/surface_mesh.h line:256
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceProperty<bool>, std::shared_ptr<easy3d::SurfaceMesh::FaceProperty<bool>>, easy3d::Property<bool>> cl(enclosing_class, "FaceProperty_bool_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::FaceProperty<bool>(); } ) );
			cl.def( pybind11::init<class easy3d::Property<bool>>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceProperty<bool> const &o){ return new easy3d::SurfaceMesh::FaceProperty<bool>(o); } ) );
			cl.def("assign", (class easy3d::SurfaceMesh::FaceProperty<bool> & (easy3d::SurfaceMesh::FaceProperty<bool>::*)(const class easy3d::SurfaceMesh::FaceProperty<bool> &)) &easy3d::SurfaceMesh::FaceProperty<bool>::operator=, "C++: easy3d::SurfaceMesh::FaceProperty<bool>::operator=(const class easy3d::SurfaceMesh::FaceProperty<bool> &) --> class easy3d::SurfaceMesh::FaceProperty<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::reset, "C++: easy3d::Property<bool>::reset() --> void");
			cl.def("data", (const bool * (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::data, "C++: easy3d::Property<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::vector, "C++: easy3d::Property<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<bool> & (easy3d::Property<bool>::*)()) &easy3d::Property<bool>::array, "C++: easy3d::Property<bool>::array() --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<bool>::*)() const) &easy3d::Property<bool>::name, "C++: easy3d::Property<bool>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<bool>::*)(const std::string &)) &easy3d::Property<bool>::set_name, "C++: easy3d::Property<bool>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<bool> & (easy3d::Property<bool>::*)(const class easy3d::Property<bool> &)) &easy3d::Property<bool>::operator=, "C++: easy3d::Property<bool>::operator=(const class easy3d::Property<bool> &) --> class easy3d::Property<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::FaceProperty file:easy3d/core/surface_mesh.h line:256
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>, std::shared_ptr<easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>>, PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_Vec_3_float_t, easy3d::Property<easy3d::Vec<3, float>>> cl(enclosing_class, "FaceProperty_easy3d_Vec_3_float_t", "");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>(); }, [](){ return new PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_Vec_3_float_t(); } ) );
			cl.def( pybind11::init<class easy3d::Property<class easy3d::Vec<3, float> >>(), pybind11::arg("p") );

			cl.def( pybind11::init( [](PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_Vec_3_float_t const &o){ return new PyCallBack_easy3d_SurfaceMesh_FaceProperty_easy3d_Vec_3_float_t(o); } ) );
			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>> const &o){ return new easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>(o); } ) );
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::*)(struct easy3d::SurfaceMesh::Face)) &easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::operator[], "C++: easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::operator[](struct easy3d::SurfaceMesh::Face) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("f"));
			cl.def("assign", (class easy3d::SurfaceMesh::FaceProperty<class easy3d::Vec<3, float> > & (easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::*)(const class easy3d::SurfaceMesh::FaceProperty<class easy3d::Vec<3, float> > &)) &easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::operator=, "C++: easy3d::SurfaceMesh::FaceProperty<easy3d::Vec<3, float>>::operator=(const class easy3d::SurfaceMesh::FaceProperty<class easy3d::Vec<3, float> > &) --> class easy3d::SurfaceMesh::FaceProperty<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
			cl.def("reset", (void (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::reset, "C++: easy3d::Property<easy3d::Vec<3, float>>::reset() --> void");
			cl.def("__getitem__", (class easy3d::Vec<3, float> & (easy3d::Property<easy3d::Vec<3, float>>::*)(size_t)) &easy3d::Property<easy3d::Vec<3, float>>::operator[], "C++: easy3d::Property<easy3d::Vec<3, float>>::operator[](size_t) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
			cl.def("data", (const class easy3d::Vec<3, float> * (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::data, "C++: easy3d::Property<easy3d::Vec<3, float>>::data() const --> const class easy3d::Vec<3, float> *", pybind11::return_value_policy::automatic);
			cl.def("vector", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::vector, "C++: easy3d::Property<easy3d::Vec<3, float>>::vector() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("array", (class easy3d::PropertyArray<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)()) &easy3d::Property<easy3d::Vec<3, float>>::array, "C++: easy3d::Property<easy3d::Vec<3, float>>::array() --> class easy3d::PropertyArray<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
			cl.def("name", (const std::string & (easy3d::Property<easy3d::Vec<3, float>>::*)() const) &easy3d::Property<easy3d::Vec<3, float>>::name, "C++: easy3d::Property<easy3d::Vec<3, float>>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
			cl.def("set_name", (void (easy3d::Property<easy3d::Vec<3, float>>::*)(const std::string &)) &easy3d::Property<easy3d::Vec<3, float>>::set_name, "C++: easy3d::Property<easy3d::Vec<3, float>>::set_name(const std::string &) --> void", pybind11::arg("n"));
			cl.def("assign", (class easy3d::Property<class easy3d::Vec<3, float> > & (easy3d::Property<easy3d::Vec<3, float>>::*)(const class easy3d::Property<class easy3d::Vec<3, float> > &)) &easy3d::Property<easy3d::Vec<3, float>>::operator=, "C++: easy3d::Property<easy3d::Vec<3, float>>::operator=(const class easy3d::Property<class easy3d::Vec<3, float> > &) --> class easy3d::Property<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::SurfaceMesh::VertexIterator file:easy3d/core/surface_mesh.h line:308
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexIterator, std::shared_ptr<easy3d::SurfaceMesh::VertexIterator>> cl(enclosing_class, "VertexIterator", "This class iterates linearly over all vertices\n \n\n vertices_begin(), vertices_end()\n \n\n HalfedgeIterator, EdgeIterator, FaceIterator");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::SurfaceMesh::Vertex const & a0){ return new easy3d::SurfaceMesh::VertexIterator(a0); } ), "doc" , pybind11::arg("v"));
			cl.def( pybind11::init<struct easy3d::SurfaceMesh::Vertex, const class easy3d::SurfaceMesh *>(), pybind11::arg("v"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexIterator const &o){ return new easy3d::SurfaceMesh::VertexIterator(o); } ) );
			cl.def("dereference", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::VertexIterator::*)() const) &easy3d::SurfaceMesh::VertexIterator::operator*, "get the vertex the iterator refers to\n\nC++: easy3d::SurfaceMesh::VertexIterator::operator*() const --> struct easy3d::SurfaceMesh::Vertex");
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::VertexIterator::*)(const class easy3d::SurfaceMesh::VertexIterator &) const) &easy3d::SurfaceMesh::VertexIterator::operator==, "are two iterators equal?\n\nC++: easy3d::SurfaceMesh::VertexIterator::operator==(const class easy3d::SurfaceMesh::VertexIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::VertexIterator::*)(const class easy3d::SurfaceMesh::VertexIterator &) const) &easy3d::SurfaceMesh::VertexIterator::operator!=, "are two iterators different?\n\nC++: easy3d::SurfaceMesh::VertexIterator::operator!=(const class easy3d::SurfaceMesh::VertexIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::VertexIterator & (easy3d::SurfaceMesh::VertexIterator::*)()) &easy3d::SurfaceMesh::VertexIterator::operator++, "pre-increment iterator\n\nC++: easy3d::SurfaceMesh::VertexIterator::operator++() --> class easy3d::SurfaceMesh::VertexIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::VertexIterator & (easy3d::SurfaceMesh::VertexIterator::*)()) &easy3d::SurfaceMesh::VertexIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::SurfaceMesh::VertexIterator::operator--() --> class easy3d::SurfaceMesh::VertexIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::SurfaceMesh::HalfedgeIterator file:easy3d/core/surface_mesh.h line:360
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::HalfedgeIterator, std::shared_ptr<easy3d::SurfaceMesh::HalfedgeIterator>> cl(enclosing_class, "HalfedgeIterator", "This class iterates linearly over all halfedges\n \n\n halfedges_begin(), halfedges_end()\n \n\n VertexIterator, EdgeIterator, FaceIterator");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::HalfedgeIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::SurfaceMesh::Halfedge const & a0){ return new easy3d::SurfaceMesh::HalfedgeIterator(a0); } ), "doc" , pybind11::arg("h"));
			cl.def( pybind11::init<struct easy3d::SurfaceMesh::Halfedge, const class easy3d::SurfaceMesh *>(), pybind11::arg("h"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::HalfedgeIterator const &o){ return new easy3d::SurfaceMesh::HalfedgeIterator(o); } ) );
			cl.def("dereference", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::HalfedgeIterator::*)() const) &easy3d::SurfaceMesh::HalfedgeIterator::operator*, "get the halfedge the iterator refers to\n\nC++: easy3d::SurfaceMesh::HalfedgeIterator::operator*() const --> struct easy3d::SurfaceMesh::Halfedge");
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::HalfedgeIterator::*)(const class easy3d::SurfaceMesh::HalfedgeIterator &) const) &easy3d::SurfaceMesh::HalfedgeIterator::operator==, "are two iterators equal?\n\nC++: easy3d::SurfaceMesh::HalfedgeIterator::operator==(const class easy3d::SurfaceMesh::HalfedgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::HalfedgeIterator::*)(const class easy3d::SurfaceMesh::HalfedgeIterator &) const) &easy3d::SurfaceMesh::HalfedgeIterator::operator!=, "are two iterators different?\n\nC++: easy3d::SurfaceMesh::HalfedgeIterator::operator!=(const class easy3d::SurfaceMesh::HalfedgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::HalfedgeIterator & (easy3d::SurfaceMesh::HalfedgeIterator::*)()) &easy3d::SurfaceMesh::HalfedgeIterator::operator++, "pre-increment iterator\n\nC++: easy3d::SurfaceMesh::HalfedgeIterator::operator++() --> class easy3d::SurfaceMesh::HalfedgeIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::HalfedgeIterator & (easy3d::SurfaceMesh::HalfedgeIterator::*)()) &easy3d::SurfaceMesh::HalfedgeIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::SurfaceMesh::HalfedgeIterator::operator--() --> class easy3d::SurfaceMesh::HalfedgeIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::SurfaceMesh::EdgeIterator file:easy3d/core/surface_mesh.h line:412
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::EdgeIterator, std::shared_ptr<easy3d::SurfaceMesh::EdgeIterator>> cl(enclosing_class, "EdgeIterator", "This class iterates linearly over all edges\n \n\n edges_begin(), edges_end()\n \n\n VertexIterator, HalfedgeIterator, FaceIterator");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::EdgeIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::SurfaceMesh::Edge const & a0){ return new easy3d::SurfaceMesh::EdgeIterator(a0); } ), "doc" , pybind11::arg("e"));
			cl.def( pybind11::init<struct easy3d::SurfaceMesh::Edge, const class easy3d::SurfaceMesh *>(), pybind11::arg("e"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::EdgeIterator const &o){ return new easy3d::SurfaceMesh::EdgeIterator(o); } ) );
			cl.def("dereference", (struct easy3d::SurfaceMesh::Edge (easy3d::SurfaceMesh::EdgeIterator::*)() const) &easy3d::SurfaceMesh::EdgeIterator::operator*, "get the edge the iterator refers to\n\nC++: easy3d::SurfaceMesh::EdgeIterator::operator*() const --> struct easy3d::SurfaceMesh::Edge");
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::EdgeIterator::*)(const class easy3d::SurfaceMesh::EdgeIterator &) const) &easy3d::SurfaceMesh::EdgeIterator::operator==, "are two iterators equal?\n\nC++: easy3d::SurfaceMesh::EdgeIterator::operator==(const class easy3d::SurfaceMesh::EdgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::EdgeIterator::*)(const class easy3d::SurfaceMesh::EdgeIterator &) const) &easy3d::SurfaceMesh::EdgeIterator::operator!=, "are two iterators different?\n\nC++: easy3d::SurfaceMesh::EdgeIterator::operator!=(const class easy3d::SurfaceMesh::EdgeIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::EdgeIterator & (easy3d::SurfaceMesh::EdgeIterator::*)()) &easy3d::SurfaceMesh::EdgeIterator::operator++, "pre-increment iterator\n\nC++: easy3d::SurfaceMesh::EdgeIterator::operator++() --> class easy3d::SurfaceMesh::EdgeIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::EdgeIterator & (easy3d::SurfaceMesh::EdgeIterator::*)()) &easy3d::SurfaceMesh::EdgeIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::SurfaceMesh::EdgeIterator::operator--() --> class easy3d::SurfaceMesh::EdgeIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::SurfaceMesh::FaceIterator file:easy3d/core/surface_mesh.h line:464
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceIterator, std::shared_ptr<easy3d::SurfaceMesh::FaceIterator>> cl(enclosing_class, "FaceIterator", "This class iterates linearly over all faces\n \n\n faces_begin(), faces_end()\n \n\n VertexIterator, HalfedgeIterator, EdgeIterator");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::FaceIterator(); } ), "doc" );
			cl.def( pybind11::init( [](struct easy3d::SurfaceMesh::Face const & a0){ return new easy3d::SurfaceMesh::FaceIterator(a0); } ), "doc" , pybind11::arg("f"));
			cl.def( pybind11::init<struct easy3d::SurfaceMesh::Face, const class easy3d::SurfaceMesh *>(), pybind11::arg("f"), pybind11::arg("m") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceIterator const &o){ return new easy3d::SurfaceMesh::FaceIterator(o); } ) );
			cl.def("dereference", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::FaceIterator::*)() const) &easy3d::SurfaceMesh::FaceIterator::operator*, "get the face the iterator refers to\n\nC++: easy3d::SurfaceMesh::FaceIterator::operator*() const --> struct easy3d::SurfaceMesh::Face");
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::FaceIterator::*)(const class easy3d::SurfaceMesh::FaceIterator &) const) &easy3d::SurfaceMesh::FaceIterator::operator==, "are two iterators equal?\n\nC++: easy3d::SurfaceMesh::FaceIterator::operator==(const class easy3d::SurfaceMesh::FaceIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::FaceIterator::*)(const class easy3d::SurfaceMesh::FaceIterator &) const) &easy3d::SurfaceMesh::FaceIterator::operator!=, "are two iterators different?\n\nC++: easy3d::SurfaceMesh::FaceIterator::operator!=(const class easy3d::SurfaceMesh::FaceIterator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::FaceIterator & (easy3d::SurfaceMesh::FaceIterator::*)()) &easy3d::SurfaceMesh::FaceIterator::operator++, "pre-increment iterator\n\nC++: easy3d::SurfaceMesh::FaceIterator::operator++() --> class easy3d::SurfaceMesh::FaceIterator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::FaceIterator & (easy3d::SurfaceMesh::FaceIterator::*)()) &easy3d::SurfaceMesh::FaceIterator::operator--, "pre-decrement iterator\n\nC++: easy3d::SurfaceMesh::FaceIterator::operator--() --> class easy3d::SurfaceMesh::FaceIterator &", pybind11::return_value_policy::automatic);
		}

		{ // easy3d::SurfaceMesh::VertexContainer file:easy3d/core/surface_mesh.h line:519
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexContainer, std::shared_ptr<easy3d::SurfaceMesh::VertexContainer>> cl(enclosing_class, "VertexContainer", "This helper class is a container for iterating through all\n vertices using C++11 range-based for-loops.\n \n\n vertices()");
			cl.def( pybind11::init<class easy3d::SurfaceMesh::VertexIterator, class easy3d::SurfaceMesh::VertexIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexContainer const &o){ return new easy3d::SurfaceMesh::VertexContainer(o); } ) );
			cl.def("begin", (class easy3d::SurfaceMesh::VertexIterator (easy3d::SurfaceMesh::VertexContainer::*)() const) &easy3d::SurfaceMesh::VertexContainer::begin, "C++: easy3d::SurfaceMesh::VertexContainer::begin() const --> class easy3d::SurfaceMesh::VertexIterator");
			cl.def("end", (class easy3d::SurfaceMesh::VertexIterator (easy3d::SurfaceMesh::VertexContainer::*)() const) &easy3d::SurfaceMesh::VertexContainer::end, "C++: easy3d::SurfaceMesh::VertexContainer::end() const --> class easy3d::SurfaceMesh::VertexIterator");

            // Liangliang: provide Python bindings to make VertexContainer iterable
            cl.def("__iter__", [](const easy3d::SurfaceMesh::VertexContainer &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::HalfedgeContainer file:easy3d/core/surface_mesh.h line:534
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::HalfedgeContainer, std::shared_ptr<easy3d::SurfaceMesh::HalfedgeContainer>> cl(enclosing_class, "HalfedgeContainer", "This helper class is a container for iterating through all\n halfedge using C++11 range-based for-loops.\n \n\n halfedges()");
			cl.def( pybind11::init<class easy3d::SurfaceMesh::HalfedgeIterator, class easy3d::SurfaceMesh::HalfedgeIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::HalfedgeContainer const &o){ return new easy3d::SurfaceMesh::HalfedgeContainer(o); } ) );
			cl.def("begin", (class easy3d::SurfaceMesh::HalfedgeIterator (easy3d::SurfaceMesh::HalfedgeContainer::*)() const) &easy3d::SurfaceMesh::HalfedgeContainer::begin, "C++: easy3d::SurfaceMesh::HalfedgeContainer::begin() const --> class easy3d::SurfaceMesh::HalfedgeIterator");
			cl.def("end", (class easy3d::SurfaceMesh::HalfedgeIterator (easy3d::SurfaceMesh::HalfedgeContainer::*)() const) &easy3d::SurfaceMesh::HalfedgeContainer::end, "C++: easy3d::SurfaceMesh::HalfedgeContainer::end() const --> class easy3d::SurfaceMesh::HalfedgeIterator");

            // Liangliang: provide Python bindings to make HalfedgeContainer iterable
             cl.def("__iter__", [](const easy3d::SurfaceMesh::HalfedgeContainer &self) {
                 return pybind11::make_iterator(self.begin(), self.end());
             }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::EdgeContainer file:easy3d/core/surface_mesh.h line:549
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::EdgeContainer, std::shared_ptr<easy3d::SurfaceMesh::EdgeContainer>> cl(enclosing_class, "EdgeContainer", "This helper class is a container for iterating through all\n edges using C++11 range-based for-loops.\n \n\n edges()");
			cl.def( pybind11::init<class easy3d::SurfaceMesh::EdgeIterator, class easy3d::SurfaceMesh::EdgeIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::EdgeContainer const &o){ return new easy3d::SurfaceMesh::EdgeContainer(o); } ) );
			cl.def("begin", (class easy3d::SurfaceMesh::EdgeIterator (easy3d::SurfaceMesh::EdgeContainer::*)() const) &easy3d::SurfaceMesh::EdgeContainer::begin, "C++: easy3d::SurfaceMesh::EdgeContainer::begin() const --> class easy3d::SurfaceMesh::EdgeIterator");
			cl.def("end", (class easy3d::SurfaceMesh::EdgeIterator (easy3d::SurfaceMesh::EdgeContainer::*)() const) &easy3d::SurfaceMesh::EdgeContainer::end, "C++: easy3d::SurfaceMesh::EdgeContainer::end() const --> class easy3d::SurfaceMesh::EdgeIterator");

            // Liangliang: provide Python bindings to make EdgeContainer iterable
             cl.def("__iter__", [](const easy3d::SurfaceMesh::EdgeContainer &self) {
                 return pybind11::make_iterator(self.begin(), self.end());
             }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::FaceContainer file:easy3d/core/surface_mesh.h line:564
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceContainer, std::shared_ptr<easy3d::SurfaceMesh::FaceContainer>> cl(enclosing_class, "FaceContainer", "This helper class is a container for iterating through all\n faces using C++11 range-based for-loops.\n \n\n faces()");
			cl.def( pybind11::init<class easy3d::SurfaceMesh::FaceIterator, class easy3d::SurfaceMesh::FaceIterator>(), pybind11::arg("_begin"), pybind11::arg("_end") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceContainer const &o){ return new easy3d::SurfaceMesh::FaceContainer(o); } ) );
			cl.def("begin", (class easy3d::SurfaceMesh::FaceIterator (easy3d::SurfaceMesh::FaceContainer::*)() const) &easy3d::SurfaceMesh::FaceContainer::begin, "C++: easy3d::SurfaceMesh::FaceContainer::begin() const --> class easy3d::SurfaceMesh::FaceIterator");
			cl.def("end", (class easy3d::SurfaceMesh::FaceIterator (easy3d::SurfaceMesh::FaceContainer::*)() const) &easy3d::SurfaceMesh::FaceContainer::end, "C++: easy3d::SurfaceMesh::FaceContainer::end() const --> class easy3d::SurfaceMesh::FaceIterator");

            // Liangliang: provide Python bindings to make FaceContainer iterable
            cl.def("__iter__", [](const easy3d::SurfaceMesh::FaceContainer &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::VertexAroundVertexCirculator file:easy3d/core/surface_mesh.h line:594
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexAroundVertexCirculator, std::shared_ptr<easy3d::SurfaceMesh::VertexAroundVertexCirculator>> cl(enclosing_class, "VertexAroundVertexCirculator", "This class circulates through all one-ring neighbors of a vertex.\n It also acts as a container-concept for C++11 range-based for loops.\n\n The follow code shows how to use VertexAroundVertexCirculator:\n \n\n\n\n\n\n\n\n\n\n \n HalfedgeAroundVertexCirculator, FaceAroundVertexCirculator, vertices()");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexAroundVertexCirculator(); } ), "doc" );
			cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0){ return new easy3d::SurfaceMesh::VertexAroundVertexCirculator(a0); } ), "doc" , pybind11::arg("m"));
			cl.def( pybind11::init<const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex>(), pybind11::arg("m"), pybind11::arg("v") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexAroundVertexCirculator const &o){ return new easy3d::SurfaceMesh::VertexAroundVertexCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)(const class easy3d::SurfaceMesh::VertexAroundVertexCirculator &) const) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator==(const class easy3d::SurfaceMesh::VertexAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)(const class easy3d::SurfaceMesh::VertexAroundVertexCirculator &) const) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator!=(const class easy3d::SurfaceMesh::VertexAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::VertexAroundVertexCirculator & (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator++, "pre-increment (rotate counter-clockwise)\n\nC++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator++() --> class easy3d::SurfaceMesh::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::VertexAroundVertexCirculator & (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator--, "pre-decrement (rotate clockwise)\n\nC++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator--() --> class easy3d::SurfaceMesh::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)() const) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator*, "get the vertex the circulator refers to\n\nC++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::operator*() const --> struct easy3d::SurfaceMesh::Vertex");
			cl.def("halfedge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)() const) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::halfedge, "return current halfedge\n\nC++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::halfedge() const --> struct easy3d::SurfaceMesh::Halfedge");
			cl.def("begin", (class easy3d::SurfaceMesh::VertexAroundVertexCirculator & (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::begin, "C++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::begin() --> class easy3d::SurfaceMesh::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::SurfaceMesh::VertexAroundVertexCirculator & (easy3d::SurfaceMesh::VertexAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundVertexCirculator::end, "C++: easy3d::SurfaceMesh::VertexAroundVertexCirculator::end() --> class easy3d::SurfaceMesh::VertexAroundVertexCirculator &", pybind11::return_value_policy::automatic);

            // Liangliang: provide Python bindings to make VertexAroundVertexCirculator iterable
            cl.def("__iter__", [](easy3d::SurfaceMesh::VertexAroundVertexCirculator &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator file:easy3d/core/surface_mesh.h line:677
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator, std::shared_ptr<easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator>> cl(enclosing_class, "HalfedgeAroundVertexCirculator", "This class circulates through all outgoing halfedges of a vertex.\n It also acts as a container-concept for C++11 range-based for loops.\n\n The follow code shows how to use HalfedgeAroundVertexCirculator:\n \n\n\n\n\n\n\n\n\n\n \n VertexAroundVertexCirculator, FaceAroundVertexCirculator, halfedges()");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator(); } ), "doc" );
			cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0){ return new easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator(a0); } ), "doc" , pybind11::arg("m"));
			cl.def( pybind11::init<const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex>(), pybind11::arg("m"), pybind11::arg("v") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator const &o){ return new easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)(const class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &) const) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator==(const class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)(const class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &) const) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator!=(const class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator & (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator++, "pre-increment (rotate counter-clockwise)\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator++() --> class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator & (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator--, "pre-decrement (rotate clockwise)\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator--() --> class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)() const) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator*, "get the halfedge the circulator refers to\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::operator*() const --> struct easy3d::SurfaceMesh::Halfedge");
			cl.def("begin", (class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator & (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::begin, "C++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::begin() --> class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator & (easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::end, "C++: easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator::end() --> class easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &", pybind11::return_value_policy::automatic);

            // Liangliang: provide Python bindings to make HalfedgeAroundVertexCirculator iterable
            cl.def("__iter__", [](easy3d::SurfaceMesh::HalfedgeAroundVertexCirculator &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::FaceAroundVertexCirculator file:easy3d/core/surface_mesh.h line:753
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::FaceAroundVertexCirculator, std::shared_ptr<easy3d::SurfaceMesh::FaceAroundVertexCirculator>> cl(enclosing_class, "FaceAroundVertexCirculator", "This class circulates through all incident faces of a vertex.\n It also acts as a container-concept for C++11 range-based for loops.\n\n The follow code shows how to use FaceAroundVertexCirculator:\n \n\n\n\n\n\n\n\n\n\n \n VertexAroundVertexCirculator, HalfedgeAroundVertexCirculator, faces()");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::FaceAroundVertexCirculator(); } ), "doc" );
			cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0){ return new easy3d::SurfaceMesh::FaceAroundVertexCirculator(a0); } ), "doc" , pybind11::arg("m"));
			cl.def( pybind11::init<const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex>(), pybind11::arg("m"), pybind11::arg("v") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::FaceAroundVertexCirculator const &o){ return new easy3d::SurfaceMesh::FaceAroundVertexCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)(const class easy3d::SurfaceMesh::FaceAroundVertexCirculator &) const) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator==(const class easy3d::SurfaceMesh::FaceAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)(const class easy3d::SurfaceMesh::FaceAroundVertexCirculator &) const) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator!=(const class easy3d::SurfaceMesh::FaceAroundVertexCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::FaceAroundVertexCirculator & (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator++, "pre-increment (rotates counter-clockwise)\n\nC++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator++() --> class easy3d::SurfaceMesh::FaceAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::FaceAroundVertexCirculator & (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator--, "pre-decrement (rotate clockwise)\n\nC++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator--() --> class easy3d::SurfaceMesh::FaceAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)() const) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator*, "get the face the circulator refers to\n\nC++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::operator*() const --> struct easy3d::SurfaceMesh::Face");
			cl.def("begin", (class easy3d::SurfaceMesh::FaceAroundVertexCirculator & (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::begin, "C++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::begin() --> class easy3d::SurfaceMesh::FaceAroundVertexCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::SurfaceMesh::FaceAroundVertexCirculator & (easy3d::SurfaceMesh::FaceAroundVertexCirculator::*)()) &easy3d::SurfaceMesh::FaceAroundVertexCirculator::end, "C++: easy3d::SurfaceMesh::FaceAroundVertexCirculator::end() --> class easy3d::SurfaceMesh::FaceAroundVertexCirculator &", pybind11::return_value_policy::automatic);

            // Liangliang: provide Python bindings to make FaceAroundVertexCirculator iterable
            cl.def("__iter__", [](easy3d::SurfaceMesh::FaceAroundVertexCirculator &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::VertexAroundFaceCirculator file:easy3d/core/surface_mesh.h line:842
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::VertexAroundFaceCirculator, std::shared_ptr<easy3d::SurfaceMesh::VertexAroundFaceCirculator>> cl(enclosing_class, "VertexAroundFaceCirculator", "This class circulates through the vertices of a face.\n It also acts as a container-concept for C++11 range-based for loops.\n\n The follow code shows how to use VertexAroundFaceCirculator:\n \n\n\n\n\n\n\n\n\n\n \n HalfedgeAroundFaceCirculator, vertices()");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::VertexAroundFaceCirculator(); } ), "doc" );
			cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0){ return new easy3d::SurfaceMesh::VertexAroundFaceCirculator(a0); } ), "doc" , pybind11::arg("m"));
			cl.def( pybind11::init<const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face>(), pybind11::arg("m"), pybind11::arg("f") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::VertexAroundFaceCirculator const &o){ return new easy3d::SurfaceMesh::VertexAroundFaceCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)(const class easy3d::SurfaceMesh::VertexAroundFaceCirculator &) const) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator==(const class easy3d::SurfaceMesh::VertexAroundFaceCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)(const class easy3d::SurfaceMesh::VertexAroundFaceCirculator &) const) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator!=(const class easy3d::SurfaceMesh::VertexAroundFaceCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::VertexAroundFaceCirculator & (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator++, "pre-increment (rotates counter-clockwise)\n\nC++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator++() --> class easy3d::SurfaceMesh::VertexAroundFaceCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::VertexAroundFaceCirculator & (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator--, "pre-decrement (rotates clockwise)\n\nC++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator--() --> class easy3d::SurfaceMesh::VertexAroundFaceCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)() const) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator*, "get the vertex the circulator refers to\n\nC++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::operator*() const --> struct easy3d::SurfaceMesh::Vertex");
			cl.def("begin", (class easy3d::SurfaceMesh::VertexAroundFaceCirculator & (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::begin, "C++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::begin() --> class easy3d::SurfaceMesh::VertexAroundFaceCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::SurfaceMesh::VertexAroundFaceCirculator & (easy3d::SurfaceMesh::VertexAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::VertexAroundFaceCirculator::end, "C++: easy3d::SurfaceMesh::VertexAroundFaceCirculator::end() --> class easy3d::SurfaceMesh::VertexAroundFaceCirculator &", pybind11::return_value_policy::automatic);

            // Liangliang: provide Python bindings to make VertexAroundFaceCirculator iterable
            cl.def("__iter__", [](easy3d::SurfaceMesh::VertexAroundFaceCirculator &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }

		{ // easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator file:easy3d/core/surface_mesh.h line:919
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator, std::shared_ptr<easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator>> cl(enclosing_class, "HalfedgeAroundFaceCirculator", "This class circulates through all halfedges of a face.\n It also acts as a container-concept for C++11 range-based for loops.\n\n The following code shows how to use HalfedgeAroundFaceCirculator:\n \n\n\n\n\n\n\n\n\n\n \n VertexAroundFaceCirculator, halfedges()");
			cl.def( pybind11::init( [](){ return new easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator(); } ), "doc" );
			cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0){ return new easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator(a0); } ), "doc" , pybind11::arg("m"));
			cl.def( pybind11::init<const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face>(), pybind11::arg("m"), pybind11::arg("f") );

			cl.def( pybind11::init( [](easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator const &o){ return new easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator(o); } ) );
			cl.def("__eq__", (bool (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)(const class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &) const) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator==, "are two circulators equal?\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator==(const class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("__ne__", (bool (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)(const class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &) const) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator!=, "are two circulators different?\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator!=(const class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &) const --> bool", pybind11::arg("rhs"));
			cl.def("pre_increment", (class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator & (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator++, "pre-increment (rotates counter-clockwise)\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator++() --> class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &", pybind11::return_value_policy::automatic);
			cl.def("pre_decrement", (class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator & (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator--, "pre-decrement (rotates clockwise)\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator--() --> class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &", pybind11::return_value_policy::automatic);
			cl.def("dereference", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)() const) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator*, "get the halfedge the circulator refers to\n\nC++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::operator*() const --> struct easy3d::SurfaceMesh::Halfedge");
			cl.def("begin", (class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator & (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::begin, "C++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::begin() --> class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &", pybind11::return_value_policy::automatic);
			cl.def("end", (class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator & (easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::*)()) &easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::end, "C++: easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator::end() --> class easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &", pybind11::return_value_policy::automatic);

            // Liangliang: provide Python bindings to make HalfedgeAroundFaceCirculator iterable
            cl.def("__iter__", [](easy3d::SurfaceMesh::HalfedgeAroundFaceCirculator &self) {
                return pybind11::make_iterator(self.begin(), self.end());
            }, pybind11::keep_alive<0, 1>());  // Keep the container alive while iterating
        }
	}

    { // easy3d::PropertyArray file:easy3d/core/property.h line:110
        pybind11::class_<easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_VertexConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_SurfaceMesh_VertexConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_VertexConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::SurfaceMesh::VertexConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_VertexConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_VertexConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::SurfaceMesh::VertexConnectivity * (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::data() const --> const struct easy3d::SurfaceMesh::VertexConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::SurfaceMesh::VertexConnectivity & (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::VertexConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> & (easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::VertexConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::VertexConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
        pybind11::class_<easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_HalfedgeConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_SurfaceMesh_HalfedgeConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_HalfedgeConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::SurfaceMesh::HalfedgeConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_HalfedgeConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_HalfedgeConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::SurfaceMesh::HalfedgeConnectivity * (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::data() const --> const struct easy3d::SurfaceMesh::HalfedgeConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::SurfaceMesh::HalfedgeConnectivity & (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::HalfedgeConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> & (easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::HalfedgeConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::HalfedgeConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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

    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<easy3d::SurfaceMesh::Halfedge>, std::shared_ptr<easy3d::Property<easy3d::SurfaceMesh::Halfedge>>, PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Halfedge_t> cl(m, "Property_easy3d_SurfaceMesh_Halfedge_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::SurfaceMesh::Halfedge>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Halfedge_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Halfedge> *>(), pybind11::arg("p") );

        cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)()) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::reset() --> void");
        cl.def("__getitem__", (struct easy3d::SurfaceMesh::Halfedge & (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::operator[](size_t) --> struct easy3d::SurfaceMesh::Halfedge &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const struct easy3d::SurfaceMesh::Halfedge * (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::data() const --> const struct easy3d::SurfaceMesh::Halfedge *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::Halfedge> & (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)()) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::vector() --> class std::vector<struct easy3d::SurfaceMesh::Halfedge> &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Halfedge> & (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)()) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Halfedge> &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::Halfedge> & (easy3d::Property<easy3d::SurfaceMesh::Halfedge>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::Halfedge> &)) &easy3d::Property<easy3d::SurfaceMesh::Halfedge>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::Halfedge>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::Halfedge> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::Halfedge> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

    { // easy3d::Property file:easy3d/core/property.h line:252
        pybind11::class_<easy3d::Property<easy3d::SurfaceMesh::Vertex>, std::shared_ptr<easy3d::Property<easy3d::SurfaceMesh::Vertex>>, PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Vertex_t> cl(m, "Property_easy3d_SurfaceMesh_Vertex_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Property<easy3d::SurfaceMesh::Vertex>(); }, [](){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Vertex_t(); } ), "doc");
        cl.def( pybind11::init<class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> *>(), pybind11::arg("p") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Vertex_t const &o){ return new PyCallBack_easy3d_Property_easy3d_SurfaceMesh_Vertex_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::Property<easy3d::SurfaceMesh::Vertex> const &o){ return new easy3d::Property<easy3d::SurfaceMesh::Vertex>(o); } ) );
        cl.def("reset", (void (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)()) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::reset, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::reset() --> void");
        cl.def("__getitem__", (struct easy3d::SurfaceMesh::Vertex & (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)(size_t)) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::operator[], "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::operator[](size_t) --> struct easy3d::SurfaceMesh::Vertex &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
        cl.def("data", (const struct easy3d::SurfaceMesh::Vertex * (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::data, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::data() const --> const struct easy3d::SurfaceMesh::Vertex *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::Vertex> & (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)()) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::vector, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::vector() --> class std::vector<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic);
        cl.def("array", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> & (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)()) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::array, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::array() --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic);
        cl.def("name", (const std::string & (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)() const) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::name, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)(const std::string &)) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::set_name, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("assign", (class easy3d::Property<struct easy3d::SurfaceMesh::Vertex> & (easy3d::Property<easy3d::SurfaceMesh::Vertex>::*)(const class easy3d::Property<struct easy3d::SurfaceMesh::Vertex> &)) &easy3d::Property<easy3d::SurfaceMesh::Vertex>::operator=, "C++: easy3d::Property<easy3d::SurfaceMesh::Vertex>::operator=(const class easy3d::Property<struct easy3d::SurfaceMesh::Vertex> &) --> class easy3d::Property<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

    { // easy3d::PropertyArray file:easy3d/core/property.h line:110
        pybind11::class_<easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>, std::shared_ptr<easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>>, PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_Vertex_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_SurfaceMesh_Vertex_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_Vertex_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::SurfaceMesh::Vertex>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_Vertex_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_Vertex_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex> const &o){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::reserve, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::resize, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::push_back, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::reset, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::swap, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::copy, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::empty_clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::type, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::SurfaceMesh::Vertex * (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::data, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::data() const --> const struct easy3d::SurfaceMesh::Vertex *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::Vertex> & (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::vector, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::vector() --> class std::vector<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::SurfaceMesh::Vertex & (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::operator[], "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::operator[](size_t) --> struct easy3d::SurfaceMesh::Vertex &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> & (easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::*)(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::operator=, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::Vertex>::operator=(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> &) --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
        pybind11::class_<easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>, std::shared_ptr<easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>>, PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_FaceConnectivity_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_easy3d_SurfaceMesh_FaceConnectivity_t", "");
        cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_FaceConnectivity_t(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, struct easy3d::SurfaceMesh::FaceConnectivity>(), pybind11::arg("name"), pybind11::arg("t") );

        cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_FaceConnectivity_t const &o){ return new PyCallBack_easy3d_PropertyArray_easy3d_SurfaceMesh_FaceConnectivity_t(o); } ) );
        cl.def( pybind11::init( [](easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity> const &o){ return new easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>(o); } ) );
        cl.def("reserve", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::reserve, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::reserve(unsigned long) --> void", pybind11::arg("n"));
        cl.def("resize", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::resize, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::resize(unsigned long) --> void", pybind11::arg("n"));
        cl.def("push_back", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::push_back, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::push_back() --> void");
        cl.def("reset", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::reset, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::reset(unsigned long) --> void", pybind11::arg("idx"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
        cl.def("transfer", (bool (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::transfer, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
        cl.def("shrink_to_fit", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::shrink_to_fit, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::shrink_to_fit() --> void");
        cl.def("swap", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::swap, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
        cl.def("copy", (void (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::copy, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
        cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::empty_clone, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
        cl.def("type", (const class std::type_info & (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::type, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
        cl.def("data", (const struct easy3d::SurfaceMesh::FaceConnectivity * (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)() const) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::data, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::data() const --> const struct easy3d::SurfaceMesh::FaceConnectivity *", pybind11::return_value_policy::automatic);
        cl.def("vector", (class std::vector<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)()) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::vector, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::vector() --> class std::vector<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic);
        cl.def("__getitem__", (struct easy3d::SurfaceMesh::FaceConnectivity & (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(size_t)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::operator[], "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::operator[](size_t) --> struct easy3d::SurfaceMesh::FaceConnectivity &", pybind11::return_value_policy::automatic, pybind11::arg("_idx"));
        cl.def("assign", (class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> & (easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::*)(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> &)) &easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::operator=, "C++: easy3d::PropertyArray<easy3d::SurfaceMesh::FaceConnectivity>::operator=(const class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> &) --> class easy3d::PropertyArray<struct easy3d::SurfaceMesh::FaceConnectivity> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
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
