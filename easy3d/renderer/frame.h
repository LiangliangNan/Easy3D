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

/** ----------------------------------------------------------
 *
 * the code is adapted from libQGLViewer with modifications.
 *		- libQGLViewer (Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#ifndef EASY3D_RENDERER_FRAME_H
#define EASY3D_RENDERER_FRAME_H

#include <easy3d/core/types.h>
#include "easy3d/util/signal.h"
#include <easy3d/renderer/constraint.h>


namespace easy3d {



    /**
     * \brief The Frame class represents a coordinate system, defined by a position and an orientation.
     * \class Frame easy3d/renderer/frame.h
     *
     * \details A Frame is a 3D coordinate system, represented by a position() and an orientation().
     * The order of these transformations is important: the Frame is first translated and then rotated around the new translated origin.
     *
     * A Frame is useful to define the position and orientation of a 3D rigid object, using its matrix() method.
     * Many functions are provided to transform a 3D point from one coordinate system (Frame) to another.
     * You may also want to transform a 3D vector (such as a normal), which corresponds to applying only the rotational part of the frame transformation.
     *
     * The translation() and the rotation() that are encapsulated in a Frame can also be used to represent a rigid transformation of space.
     * Such a transformation can also be interpreted as a change of coordinate system.
     *
     * <h3>Hierarchy of Frames</h3>
     * The position and the orientation of a Frame are actually defined with respect to a referenceFrame().
     * The default referenceFrame() is the world coordinate system (represented by a NULL referenceFrame()).
     * Frames can hence easily be organized in a tree hierarchy, which root is the world coordinate system.
     *
     * <h3>Constraints</h3>
     * An interesting feature of Frames is that their displacements can be constrained.
     * When a Constraint is attached to a Frame, it filters the input of translate() and rotate(), and only the resulting filtered motion is applied to the Frame.
     * Constraints are especially useful for the ManipulatedFrame instances, in order to forbid some mouse motions.
     *
     * <h3>Derived classes</h3>
     * The ManipulatedFrame class inherits Frame and implements a mouse motion conversion, so that a Frame (and hence an object) can be manipulated in the scene with the mouse.
     */
    class Frame {
    public:
        /**
         * \brief Default constructor.
         * \details Initializes the Frame object with default values.
         */
        Frame();
        /*! Virtual destructor. Empty. */
        virtual ~Frame();

        /**
         * \brief Copy constructor.
         * \param frame The Frame object to copy from.
         * \details Creates a new Frame object as a copy of the given Frame object.
         */
        Frame(const Frame &frame);
        /**
         * \brief Assignment operator.
         * \param frame The Frame object to assign from.
         * \return A reference to this Frame object.
         * \details Performs a deep copy of all member variables from the given Frame object.
         */
        Frame &operator=(const Frame &frame);

    public:
        /*! @name World coordinates position and orientation */
        //@{

        /**
         * \brief Constructor with position and orientation.
         * \param position The position of the Frame.
         * \param orientation The orientation of the Frame.
         * \details Initializes the Frame object with the given position and orientation.
         */
        Frame(const vec3 &position, const quat &orientation);

        /**
         * \brief Sets the position of the Frame.
         * \param position The new position of the Frame.
         */
        void setPosition(const vec3 &position);
        /**
         * \brief Sets the position of the Frame with constraint.
         * \param position The new position of the Frame.
         */
        void setPositionWithConstraint(vec3 &position);
        /**
         * \brief Sets the orientation of the Frame.
         * \param orientation The new orientation of the Frame.
         */
        void setOrientation(const quat &orientation);
        /**
         * \brief Sets the orientation of the Frame with constraint.
         * \param orientation The new orientation of the Frame.
         */
        void setOrientationWithConstraint(quat &orientation);
        /**
         * \brief Sets the position and orientation of the Frame.
         * \param position The new position of the Frame.
         * \param orientation The new orientation of the Frame.
         */
        void setPositionAndOrientation(const vec3 &position, const quat &orientation);
        /**
         * \brief Sets the position and orientation of the Frame with constraint.
         * \param position The new position of the Frame.
         * \param orientation The new orientation of the Frame.
         */
        void setPositionAndOrientationWithConstraint(vec3 &position, quat &orientation);
        /**
         * \brief Returns the position of the Frame.
         * \return The position of the Frame.
         */
        vec3 position() const;
        /**
         * \brief Returns the orientation of the Frame.
         * \return The orientation of the Frame.
         */
        quat orientation() const;

        //@}

    public:
        /*! @name Local translation and rotation w/r reference Frame */
        //@{

        /**
         * \brief Sets the translation of the Frame.
         * \param translation The new translation of the Frame.
         * \details Sets the translation of the Frame, locally defined with respect to the referenceFrame().
         *      Emits the modified() signal.
         */
        void setTranslation(const vec3 &translation) {
            t_ = translation;
            modified.send();
        }
        /**
         * \brief Sets the translation of the Frame with constraint.
         * \param translation The new translation of the Frame.
         */
        void setTranslationWithConstraint(vec3 &translation);

        /**
         * \brief Sets the rotation of the Frame.
         * \param rotation The new rotation of the Frame.
         * \details Sets the rotation of the Frame, locally defined with respect to the referenceFrame().
         * Emits the modified() signal.
         */
        void setRotation(const quat &rotation) {
            q_ = rotation;
            modified.send();
        }
        /**
         * \brief Sets the rotation of the Frame with constraint.
         * \param rotation The new rotation of the Frame.
         */
        void setRotationWithConstraint(quat &rotation);
        /**
         * \brief Sets the translation and rotation of the Frame.
         * \param translation The new translation of the Frame.
         * \param rotation The new rotation of the Frame.
         */
        void setTranslationAndRotation(const vec3 &translation, const quat &rotation);
        /**
         * \brief Sets the translation and rotation of the Frame with constraint.
         * \param translation The new translation of the Frame.
         * \param rotation The new rotation of the Frame.
         */
        void setTranslationAndRotationWithConstraint(vec3 &translation, quat &rotation);

        /**
         * \brief Returns the translation of the Frame.
         * \return The translation of the Frame.
         * \details Returns the Frame translation, defined with respect to the referenceFrame().
         * Use position() to get the result in the world coordinates.
         */
        vec3 translation() const { return t_; }
        /**
         * \brief Returns the rotation of the Frame.
         * \return The rotation of the Frame.
         * \details Returns the Frame rotation, defined with respect to the referenceFrame().
         * Use orientation() to get the result in the world coordinates.
         */
        quat rotation() const { return q_; }

    public:
        /*! @name Frame hierarchy */
        //@{
        /**
         * \brief Returns the reference Frame.
         * \return The reference Frame.
         * \details Returns the reference Frame, in which coordinates system the Frame is defined.
         *      The translation() and rotation() of the Frame are defined with respect to the referenceFrame()
         *      coordinate system. A \c NULL referenceFrame() (default value)means that the Frame is defined in
         *      the world coordinate system.
         *      Use position() and orientation() to recursively convert values along the referenceFrame() chain
         *      and to get values expressed in the world coordinate system. The values match when the
         *      referenceFrame() is \c NULL.
         *      Use setReferenceFrame() to set this value and create a Frame hierarchy. Convenient functions allow you
         *      to convert 3D coordinates from one Frame to another: see coordinatesOf(), localCoordinatesOf(),
         *      coordinatesOfIn() and their inverse functions.
         *      Vectors can also be converted using transformOf(), transformOfIn, localTransformOf() and their inverse
         *      functions.
         */
        const Frame *referenceFrame() const { return referenceFrame_; }
        /**
         * \brief Sets the reference Frame.
         * \details The Frame translation() and rotation() are then defined in the referenceFrame() coordinate system.
         *      Use position() and orientation() to express these in the world coordinate system.
         * \param refFrame The reference Frame.
         */
        void setReferenceFrame(const Frame* refFrame);
        /**
         * \brief Checks if setting the reference Frame will create a loop.
         * \param frame The Frame to check.
         * \return true if setting the reference Frame will create a loop, false otherwise.
         */
        bool settingAsReferenceFrameWillCreateALoop(const Frame* frame);
        //@}

        /*! @name Frame modification */
        //@{

        /**
         * \brief Translates the Frame.
         * \param t The translation vector.
         */
        void translate(vec3 &t);
        /**
         * \brief Translates the Frame.
         * \param t The translation vector.
         */
        void translate(const vec3 &t);
        /**
         * \brief Rotates the Frame.
         * \param q The rotation quaternion.
         */
        void rotate(quat &q);
        /**
         * \brief Rotates the Frame.
         * \param q The rotation quaternion.
         */
        void rotate(const quat &q);
        /**
         * \brief Rotates the Frame around a point.
         * \param rotation The rotation quaternion.
         * \param point The point to rotate around.
         */
        void rotateAroundPoint(quat &rotation, const vec3 &point);
        /**
         * \brief Rotates the Frame around a point.
         * \param rotation The rotation quaternion.
         * \param point The point to rotate around.
         */
        void rotateAroundPoint(const quat &rotation, const vec3 &point);
        /**
         * \brief Aligns the Frame with another Frame.
         * \param frame The Frame to align with.
         * \param move Whether to move the Frame.
         * \param threshold The threshold for alignment.
         */
        void alignWithFrame(const Frame *const frame, bool move = false, float threshold = 0.0f);
        /**
         * \brief Projects the Frame on a line.
         * \param origin The origin of the line.
         * \param direction The direction of the line.
         */
        void projectOnLine(const vec3 &origin, const vec3 &direction);
        //@}

        /*! @name Coordinate system transformation of 3D coordinates */
        //@{

        /**
         * \brief Transforms a 3D point to the Frame's coordinate system.
         * \param src The source point.
         * \return The transformed point.
         */
        vec3 coordinatesOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D point from the Frame's coordinate system to the world coordinate system.
         * \param src The source point.
         * \return The transformed point.
         */
        vec3 inverseCoordinatesOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D point to the Frame's local coordinate system.
         * \param src The source point.
         * \return The transformed point.
         */
        vec3 localCoordinatesOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D point from the Frame's local coordinate system to the world coordinate system.
         * \param src The source point.
         * \return The transformed point.
         */
        vec3 localInverseCoordinatesOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D point to another Frame's coordinate system.
         * \param src The source point.
         * \param in The target Frame.
         * \return The transformed point.
         */
        vec3 coordinatesOfIn(const vec3 &src, const Frame *const in) const;
        /**
         * \brief Transforms a 3D point from another Frame's coordinate system to the Frame's coordinate system.
         * \param src The source point.
         * \param from The source Frame.
         * \return The transformed point.
         */
        vec3 coordinatesOfFrom(const vec3 &src, const Frame *const from) const;
        //@}

        /*! @name Coordinate system transformation of vectors */
        // A frame is as a new coordinate system, defined with respect to a reference
        // frame (the world coordinate system by default, see the "Composition of
        // frame" section).

        // The transformOf() (resp. inverseTransformOf()) functions transform a 3D
        // vector from (resp. to) the world coordinate system. This section defines
        // the 3D vector transformation functions. See the Coordinate system
        // transformation of 3D points above for the transformation of 3D points. The
        // difference between the two sets of functions is simple: for vectors, only
        // the rotational part of the transformations is taken into account, while
        // translation is also considered for 3D points.

        // The length of the resulting transformed vector is identical to the one of
        // the source vector for all the described functions.

        // When local is prepended to the names of the functions, the functions simply
        // transform from (and to) the reference frame.

        // When In (resp. From) is appended to the names, the functions transform from
        // (resp. To) the frame that is given as an argument. The frame does not need
        // to be in the same branch or the hierarchical tree, and can be \c NULL (the
        // world coordinates system).

        // Combining any of these functions with its inverse (in any order) leads to
        // the identity.
        //@{
        /**
         * \brief Transforms a 3D vector to the Frame's coordinate system.
         * \param src The source vector.
         * \return The transformed vector.
         */
        vec3 transformOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D vector from the Frame's coordinate system to the world coordinate system.
         * \param src The source vector.
         * \return The transformed vector.
         */
        vec3 inverseTransformOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D vector to the Frame's local coordinate system.
         * \param src The source vector.
         * \return The transformed vector.
         */
        vec3 localTransformOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D vector from the Frame's local coordinate system to the world coordinate system.
         * \param src The source vector.
         * \return The transformed vector.
         */
        vec3 localInverseTransformOf(const vec3 &src) const;
        /**
         * \brief Transforms a 3D vector to another Frame's coordinate system.
         * \param src The source vector.
         * \param in The target Frame.
         * \return The transformed vector.
         */
        vec3 transformOfIn(const vec3 &src, const Frame *const in) const;
        /**
         * \brief Transforms a 3D vector from another Frame's coordinate system to the Frame's coordinate system.
         * \param src The source vector.
         * \param from The source Frame.
         * \return The transformed vector.
         */
        vec3 transformOfFrom(const vec3 &src, const Frame *const from) const;
        //@}

        /*! @name Constraint on the displacement */
        //@{
        /**
         * \brief Returns the current constraint applied to the Frame.
         * \return The current constraint.
         * \details A NULL value (default) means that no Constraint is used to filter Frame translation and rotation.
         */
        Constraint *constraint() const { return constraint_; }

        /**
         * \brief Sets the constraint attached to the Frame.
         * \param constraint The new constraint.
         * \details A NULL value means no constraint. The previous constraint should be deleted by the calling method if needed.
         */
        void setConstraint(Constraint *const constraint) { constraint_ = constraint; }
        //@}


        /*! @name Associated matrices */
        //@{
    public:
        /**
         * \brief Returns the transformation matrix of the Frame.
         * \return The transformation matrix.
         */
        mat4 matrix() const;
        /**
         * \brief Returns the world transformation matrix of the Frame.
         * \return The world transformation matrix.
         */
        mat4 worldMatrix() const;
        /**
         * \brief Sets the Frame from a transformation matrix.
         * \param m The transformation matrix.
         */
        void setFromMatrix(const mat4 &m);

        //@}

        /*! @name Inversion of the transformation */
        //@{
        /**
         * \brief Returns the inverse of the Frame.
         * \return The inverse of the Frame.
         * \sa worldInverse()
         */
        Frame inverse() const;

        /**
         * \brief Returns the inverse of the Frame's world transformation.
         * \return The inverse of the Frame's world transformation.
         * \details The result Frame has a NULL referenceFrame() and a NULL constraint(). The orientation() of the new
         *      Frame is the quat::inverse() of the original orientation. Its position() is the negated and inverse
         *      rotated image of the original position.
         *      Use inverse() for a local (i.e. with respect to referenceFrame()) transformation inverse.
         * \sa inverse()
         */
        Frame worldInverse() const {
            return Frame(-(orientation().inverse_rotate(position())), orientation().inverse());
        }
        //@}

    private:
        // Position and orientation
        vec3 t_;    ///< Translation vector
        quat q_;    ///< Rotation quaternion

        // Constraints
        Constraint *constraint_;    ///< Constraint applied to the Frame

        // Frame composition
        const Frame *referenceFrame_;   ///< Reference Frame

    public:
        Signal<> modified;  ///< Signal emitted when the Frame is modified
    };

}

#endif // EASY3D_RENDERER_FRAME_H
