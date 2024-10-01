/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lightning.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obrittne <obrittne@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/29 14:44:56 by obrittne          #+#    #+#             */
/*   Updated: 2024/10/01 19:23:00 by obrittne         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minirt.h"

t_vec3	random_vector(void)
{
	t_vec3	res;

	res.x = ((double)rand() / (double)RAND_MAX - 0.5) * 2.0;
	res.y = ((double)rand() / (double)RAND_MAX - 0.5) * 2.0;
	res.z = ((double)rand() / (double)RAND_MAX - 0.5) * 2.0;
	return (normalize(res));
}

int	is_in_shadow(t_data *data, t_hit *hit, t_vec3 *light_dir, double dist)
{
	t_ray	ray;
	t_hit	new_hit;


	ray.ray_direction = *light_dir;
	ray.ray_origin = add(hit->world_position, scale(hit->world_normal, 0.00001));
	ray_trace(data, &ray, &new_hit);
	if (new_hit.found == 1 && new_hit.hit_distance < dist)
	{
		return (1);
	}
	return (0);
}

typedef struct s_calculate_light
{
	t_vec3	effective_color;
	t_vec3	ambitient;
	t_vec3	light_dir;
	t_vec3	difuse;
	t_vec3	specular;
	double	light_dot;
	t_vec3	reflect;
	double	reflect_dot;
	double	factor;
}	t_calculate_light;

// void	calculate_light2(t_data *data, t_ray *ray, t_hit *hit,
// 		t_calculate_light *cl)
// {
// 	cl->difuse = scale(cl->effective_color, cl->light_dot);
// 	cl->reflect = vec_reflect_norm(scale(cl->light_dir, -1.0), \
// 		hit->world_normal);
// 	cl->reflect_dot = dot_product(cl->reflect, ray->ray_direction);
// 	if (cl->reflect_dot > 0)
// 	{
// 		cl->factor = pow(cl->reflect_dot, 10);
// 		cl->specular = scale(data->light[0].vec3_color, \
// 			data->light[0].ratio * cl->factor);
// 	}
// }

typedef struct s_matrix4
{
	double	cords[4][4];
}	t_matrix4;

t_matrix4	create_scaling_matrix(double x, double y, double z)
{
	t_matrix4	mat;
	int			_y;
	int			_x;

	_y = 0;
	while (_y < 4)
	{
		_x = 0;
		while (_x < 4)
		{
			mat.cords[_y][_x] = 0;
			_x++;
		}
		_y++;
	}
	mat.cords[0][0] = x;
	mat.cords[1][1] = y;
	mat.cords[2][2] = z;
	mat.cords[3][3] = 1;
	return (mat);
}

t_matrix4	create_translating_matrix(double x, double y, double z)
{
	t_matrix4	mat;
	int			_y;
	int			_x;

	_y = 0;
	while (_y < 4)
	{
		_x = 0;
		while (_x < 4)
		{
			mat.cords[_y][_x] = 0;
			_x++;
		}
		_y++;
	}
	mat.cords[0][0] = 1;
	mat.cords[1][1] = 1;
	mat.cords[2][2] = 1;
	mat.cords[0][3] = x;
	mat.cords[1][3] = y;
	mat.cords[2][3] = z;
	mat.cords[3][3] = 1;
	return (mat);
}

double determinant(t_matrix4 m) {
    double det = 0;
    // Calculate the determinant using cofactor expansion
    for (int i = 0; i < 4; i++) {
        double submatrix[3][3];
        for (int j = 1; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                if (k < i) {
                    submatrix[j - 1][k] = m.cords[j][k];
                } else if (k > i) {
                    submatrix[j - 1][k - 1] = m.cords[j][k];
                }
            }
        }
        double sub_det = submatrix[0][0] * (submatrix[1][1] * submatrix[2][2] - submatrix[1][2] * submatrix[2][1])
                       - submatrix[0][1] * (submatrix[1][0] * submatrix[2][2] - submatrix[1][2] * submatrix[2][0])
                       + submatrix[0][2] * (submatrix[1][0] * submatrix[2][1] - submatrix[1][1] * submatrix[2][0]);
        det += (i % 2 == 0 ? 1 : -1) * m.cords[0][i] * sub_det;
    }
    return det;
}

t_matrix4 adjugate(t_matrix4 m) {
	t_matrix4 adj;
	// Calculate the adjugate matrix
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			double submatrix[3][3];
			int subi = 0;
			for (int row = 0; row < 4; row++) {
				if (row == i) continue;
				int subj = 0;
				for (int col = 0; col < 4; col++) {
					if (col == j) continue;
					submatrix[subi][subj] = m.cords[row][col];
					subj++;
				}
				subi++;
			}
			// Calculate the determinant of the 3x3 submatrix
			double sub_det = submatrix[0][0] * (submatrix[1][1] * submatrix[2][2] - submatrix[1][2] * submatrix[2][1])
							- submatrix[0][1] * (submatrix[1][0] * submatrix[2][2] - submatrix[1][2] * submatrix[2][0])
							+ submatrix[0][2] * (submatrix[1][0] * submatrix[2][1] - submatrix[1][1] * submatrix[2][0]);
			adj.cords[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * sub_det; // Transpose and apply sign
		}
	}
	return adj;
}

t_matrix4 inverse(t_matrix4 m) {
    t_matrix4 inv;
    double det = determinant(m);
    if (det == 0) {
        // Return an identity matrix or handle the error as needed
        printf("Matrix is not invertible\n");
		inv.cords[0][0] = 0;
        return (inv);
    }
    t_matrix4 adj = adjugate(m);
    // Multiply adjugate by 1/det to get the inverse
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            inv.cords[i][j] = adj.cords[i][j] / det;
        }
    }
    return inv;
}

t_vec3	multiply_mat_vec3(t_matrix4 m, t_vec3 p)
{
	t_vec3	result;
	double	w;

	result.x = m.cords[0][0] * p.x + m.cords[0][1] * p.y + m.cords[0][2] * p.z + m.cords[0][3];
    result.y = m.cords[1][0] * p.x + m.cords[1][1] * p.y + m.cords[1][2] * p.z + m.cords[1][3];
    result.z = m.cords[2][0] * p.x + m.cords[2][1] * p.y + m.cords[2][2] * p.z + m.cords[2][3];
	w = m.cords[3][0] * p.x + m.cords[3][1] * p.y + m.cords[3][2] * p.z + m.cords[3][3];
	if (w != 0)
	{
		result = scale(result, 1.0 / w);
	}
	return result;
}

// double u = 0.5 + atan2(vec.z, vec.x) / 2.0 / M_PI;
// double v = 0.5 + asin(vec.y);



t_vec3	checker_plane(t_hit *hit)
{
	double	u;
	double	v;

	get_uv_plane(hit, &u, &v);
	// dprintf(1, "%f\n", hit->plane->dist);
	if (((long long)floor(u / pow(hit->plane->dist, 0.75)) + (long long)floor(v / pow(hit->plane->dist, 0.75))) % 2 == 0)
		return (create_vec3(1, 1, 1));
	return (hit->color);
}

t_vec3	stripe_at(t_vec3 point, t_vec3 color1, t_vec3 color2)
{
	if (((long long)floor(point.x) +(long long)floor(point.y) + (long long)floor(point.z)) % 2)
		return color1;
	return color2;
}

// t_vec3	checker_sphere(t_hit *hit)
// {
//     double theta, phi;
//     double u, v;
//     // double checker_scale = 25.0;  // Adjust the size of the checker squares

//     // Spherical coordinates from normal vector
//     theta = atan2(hit->world_normal.y, hit->world_normal.x);  // Range: [-PI, PI]
//     if (theta < 0)
//         theta += 2.0 * M_PI;  // Adjust theta to [0, 2*PI]

//     phi = acos(hit->world_normal.z);  // Range: [0, PI]

//     // UV coordinates
//     u = theta / (2.0 * M_PI);  // Map theta to [0, 1]
//     v = phi / M_PI;            // Map phi to [0, 1]

//     // To reduce distortion near the poles, you can adjust the UV coordinates
//     // by multiplying `v` with a correction factor, making the pattern denser near the poles.
//     v = pow(v, 0.5);  // This flattens the v-coordinates near the poles

//     // Apply checkerboard pattern
//     if (((long long)(floor(u * 60.0) + floor(v * 37.5))) % 2 == 0)
//         return create_vec3(1, 1, 1);  // White color for even squares
//     else
//         return hit->color;   
// }


t_vec3 checker_sphere(t_hit *hit)
{
	double	theta;
	double	phi;
	int		lat_band;
	int		lon_band;

	theta = atan2(hit->world_normal.y, hit->world_normal.x);
	if (theta < 0)
		theta += 2.0 * M_PI;
	phi = acos(hit->world_normal.z);
	phi = pow(phi, 0.7);
	lat_band = (int)(phi / M_PI * 30);
	lon_band = (int)(theta / (2.0 * M_PI) * 20);
	if ((lat_band + lon_band) % 2 == 0)
		return create_vec3(1, 1, 1);
	else
		return hit->color;
}



t_vec3	stripe_at_object(t_hit *hit)
{
	if (hit->type == 2)
	{
		// return (hit->color);
		return (checker_plane(hit));
	}
	if (hit->type == 1)
	{
		return (checker_sphere(hit));	
	}
	return (hit->color);
	// return (stripe_at(pattern_point, hit->color, create_vec3(1.0, 1.0, 1.0)));
}


t_vec3	manage_textures(t_hit *hit)
{
	if (hit->type == 1)
		return (apply_texture_sphere(hit));	
	if (hit->type == 2)
		return (apply_texture_plane(hit));
	return (hit->color);
}

t_vec3	calculate_light(t_data *data, t_ray *ray, t_hit *hit)
{
	double				dist;
	t_calculate_light	cl;
	int					i;
	t_vec3				final;
	t_vec3				color;

	color = hit->color;
	if (hit->checkers)
	{
		color = stripe_at_object(hit);
		
		// return (color);
	}
	else if (hit->texture)
	{
		color = manage_textures(hit);
	}

	(void)ray;
	cl.ambitient = scale(multiply_vec3(color, \
		data->ambitient_light.vec3_color), data->ambitient_light.ratio);

	final = create_vec3(0, 0, 0);
	i = 0;
	int	t;

	i = 0;
	while (i < data->amount_of_lights)
	{
		cl.effective_color = multiply_vec3(color, \
		scale(data->light[i].vec3_color, data->light[i].ratio));
		cl.difuse = create_vec3(0, 0, 0);
		cl.specular = create_vec3(0, 0, 0);
		cl.light_dir = subtract(data->light[i].vec3_cords, hit->world_position);
		dist = sqrt(dot_product(cl.light_dir, cl.light_dir));
		cl.light_dir = normalize(cl.light_dir);
		cl.light_dot = dot_product(cl.light_dir, hit->world_normal);
		t = 0;
		if (cl.light_dot > 0 && !is_in_shadow(data, hit, &cl.light_dir, dist))
		{
			cl.difuse = scale(cl.effective_color, cl.light_dot);
			cl.reflect = vec_reflect_norm(scale(cl.light_dir, -1.0), \
				hit->world_normal);
			cl.reflect_dot = dot_product(cl.reflect, ray->ray_direction);
			if (cl.reflect_dot > 0)
			{
				cl.factor = pow(cl.reflect_dot, 10);
				cl.specular = scale(data->light[i].vec3_color, \
					data->light[i].ratio * cl.factor);
			}
		}
		final = add(final, add(cl.difuse, cl.specular));
		i++;
	}
	return (add(final, cl.ambitient));
}
