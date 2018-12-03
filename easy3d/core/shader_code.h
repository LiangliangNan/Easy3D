#ifndef _EASY3D_OPENGL_SHADER_CODES_H_
#define _EASY3D_OPENGL_SHADER_CODES_H_


#include <string>


namespace easy3d {

	namespace shadercode {

		extern std::string lines_color_frag;
		extern std::string lines_color_vert;

		extern std::string surface_color_frag;
		extern std::string surface_color_vert;
	}

}

/*
#define		average_color_final		"average_color_final"
extern std::string average_color_final_frag;
extern std::string average_color_final_vert;

#define		average_color_init		"average_color_init"
extern std::string average_color_init_frag;
extern std::string average_color_init_vert;

#define		basic_lighting		"basic_lighting"
extern std::string basic_lighting_frag;
extern std::string basic_lighting_vert;

#define		dual_peeling_blend		"dual_peeling_blend"
extern std::string dual_peeling_blend_frag;
extern std::string dual_peeling_blend_vert;

#define		dual_peeling_final		"dual_peeling_final"
extern std::string dual_peeling_final_frag;
extern std::string dual_peeling_final_vert;

#define		dual_peeling_init		"dual_peeling_init"
extern std::string dual_peeling_init_frag;
extern std::string dual_peeling_init_vert;

#define		dual_peeling_peel_color		"dual_peeling_peel_color"
extern std::string dual_peeling_peel_color_frag;
extern std::string dual_peeling_peel_color_vert;

#define		dual_peeling_peel_texture		"dual_peeling_peel_texture"
extern std::string dual_peeling_peel_texture_frag;
extern std::string dual_peeling_peel_texture_vert;

#define		edl_bilateral_filter		"edl_bilateral_filter"
extern std::string edl_bilateral_filter_frag;
extern std::string edl_bilateral_filter_vert;

#define		edl_compose		"edl_compose"
extern std::string edl_compose_frag;
extern std::string edl_compose_vert;

#define		edl_shade		"edl_shade"
extern std::string edl_shade_frag;
extern std::string edl_shade_vert;

#define		lines_cones_color		"lines_cones_color"
extern std::string lines_cones_color_frag;
extern std::string lines_cones_color_geom;
extern std::string lines_cones_color_vert;

#define		lines_cones_texture		"lines_cones_texture"
extern std::string lines_cones_texture_frag;
extern std::string lines_cones_texture_geom;
extern std::string lines_cones_texture_vert;

#define		lines_cylinders_color		"lines_cylinders_color"
extern std::string lines_cylinders_color_frag;
extern std::string lines_cylinders_color_geom;
extern std::string lines_cylinders_color_vert;

#define		lines_cylinders_texture		"lines_cylinders_texture"
extern std::string lines_cylinders_texture_frag;
extern std::string lines_cylinders_texture_geom;
extern std::string lines_cylinders_texture_vert;


#define		lines_plain_texture		"lines_plain_texture"
extern std::string lines_plain_texture_frag;
extern std::string lines_plain_texture_vert;

#define		mesh_effect_glass_absorption		"mesh_effect_glass_absorption"
extern std::string mesh_effect_glass_absorption_frag;
extern std::string mesh_effect_glass_absorption_vert;

#define		mesh_effect_glass_depth		"mesh_effect_glass_depth"
extern std::string mesh_effect_glass_depth_frag;
extern std::string mesh_effect_glass_depth_vert;

#define		mesh_effect_toon		"mesh_effect_toon"
extern std::string mesh_effect_toon_frag;
extern std::string mesh_effect_toon_vert;

#define		pointcloud_group		"pointcloud_group"
extern std::string pointcloud_group_frag;
extern std::string pointcloud_group_vert;

#define		points_plain_color		"points_plain_color"
extern std::string points_plain_color_frag;
extern std::string points_plain_color_vert;

#define		points_plain_texture		"points_plain_texture"
extern std::string points_plain_texture_frag;
extern std::string points_plain_texture_vert;

#define		points_spheres_geometry_color		"points_spheres_geometry_color"
extern std::string points_spheres_geometry_color_frag;
extern std::string points_spheres_geometry_color_geom;
extern std::string points_spheres_geometry_color_vert;

#define		points_spheres_geometry_texture		"points_spheres_geometry_texture"
extern std::string points_spheres_geometry_texture_frag;
extern std::string points_spheres_geometry_texture_geom;
extern std::string points_spheres_geometry_texture_vert;

#define		points_spheres_sprite_color		"points_spheres_sprite_color"
extern std::string points_spheres_sprite_color_frag;
extern std::string points_spheres_sprite_color_vert;

#define		points_spheres_sprite_texture		"points_spheres_sprite_texture"
extern std::string points_spheres_sprite_texture_frag;
extern std::string points_spheres_sprite_texture_vert;

#define		selection_model		"selection_model"
extern std::string selection_model_frag;
extern std::string selection_model_vert;

#define		selection_model_pointcloud_sphere_sprite		"selection_model_pointcloud_sphere_sprite"
extern std::string selection_model_pointcloud_sphere_sprite_frag;
extern std::string selection_model_pointcloud_sphere_sprite_vert;

#define		selection_pointcloud_lasso		"selection_pointcloud_lasso"
extern std::string selection_pointcloud_lasso_frag;
extern std::string selection_pointcloud_lasso_vert;

#define		selection_pointcloud_rect		"selection_pointcloud_rect"
extern std::string selection_pointcloud_rect_frag;
extern std::string selection_pointcloud_rect_vert;

#define		selection_point_sphere_sprite		"selection_point_sphere_sprite"
extern std::string selection_point_sphere_sprite_frag;
extern std::string selection_point_sphere_sprite_vert;

#define		selection_single_primitive		"selection_single_primitive"
extern std::string selection_single_primitive_frag;
extern std::string selection_single_primitive_vert;

#define		shadow_lines_cones		"shadow_lines_cones"
extern std::string shadow_lines_cones_frag;
extern std::string shadow_lines_cones_geom;
extern std::string shadow_lines_cones_vert;

#define		shadow_lines_cylinders		"shadow_lines_cylinders"
extern std::string shadow_lines_cylinders_frag;
extern std::string shadow_lines_cylinders_geom;
extern std::string shadow_lines_cylinders_vert;

#define		shadow_lines_plain		"shadow_lines_plain"
extern std::string shadow_lines_plain_frag;
extern std::string shadow_lines_plain_vert;

#define		shadow_points_plain		"shadow_points_plain"
extern std::string shadow_points_plain_frag;
extern std::string shadow_points_plain_vert;

#define		shadow_points_plain_texture		"shadow_points_plain_texture"
extern std::string shadow_points_plain_texture_frag;
extern std::string shadow_points_plain_texture_vert;

#define		shadow_points_spheres_geometry		"shadow_points_spheres_geometry"
extern std::string shadow_points_spheres_geometry_frag;
extern std::string shadow_points_spheres_geometry_geom;
extern std::string shadow_points_spheres_geometry_vert;

#define		shadow_surface		"shadow_surface"
extern std::string shadow_surface_frag;
extern std::string shadow_surface_vert;

#define		ssao		"ssao"
extern std::string ssao_frag;
extern std::string ssao_vert;

#define		ssao_blur		"ssao_blur"
extern std::string ssao_blur_frag;
extern std::string ssao_blur_vert;

#define		ssao_geometry		"ssao_geometry"
extern std::string ssao_geometry_frag;
extern std::string ssao_geometry_vert;

#define		surface_texture		"surface_texture"
extern std::string surface_texture_frag;
extern std::string surface_texture_vert;

#define		textured_quad		"textured_quad"
extern std::string textured_quad_frag;
extern std::string textured_quad_vert;


*/
#endif // _EASY3D_OPENGL_SHADER_CODES_H_
