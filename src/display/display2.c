/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qdo <qdo@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/29 16:34:46 by qdo               #+#    #+#             */
/*   Updated: 2024/10/02 22:36:27 by qdo              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minirt.h"

void	mem_set_0(void *a, size_t len)
{
	size_t	i;
	char	*b;

	b = (char *)a;
	i = 0;
	while (i < len)
	{
		b[i] = 0;
		i++;
	}
}

uint32_t	per_pixel(t_data *data, t_ray *ray, uint32_t x, uint32_t y)
{
	t_hit	hit;
	t_vec3	color;

	ray->ray_direction = get_direction_ray(data, ((double)x / \
	(double)data->image->width) * 2.0 - 1.0, (1.0 - (double)y / \
	(double)data->image->height) * 2.0 - 1.0);
	mem_set_0(&hit, sizeof(t_hit));
	ray_trace(data, ray, &hit);
	if (hit.found == 0)
		return (255);
	color = calculate_light(data, ray, &hit);
	return (get_color_from_vec3(shrink_vec3(color, 0.0, 1.0)));
}

int	displaying(t_data *data)
{
	uint32_t	y;
	uint32_t	x;
	uint32_t	pixel;
	t_ray		ray;

	y = 0;
	while (y < data->image->height)
	{
		x = 0;
		while (x < data->image->width)
		{
			ray.ray_origin = create_vec3_arr(data->camera.cords);
			pixel = per_pixel(data, &ray, x, y);
			mlx_put_pixel(data->image, x, y, pixel);
			x++;
		}
		y++;
	}
	return (1);
}

void	display(t_data *data)
{
	data->mlx = mlx_init(WIDTH, HEIGHT, "Scene", true);
	if (!data->mlx)
		return (display_error_message("Couldnt init window"));
	data->image = mlx_new_image(data->mlx, WIDTH, HEIGHT);
	if (!data->image)
		return (display_error_message("Couldnt create image"), \
		mlx_close_window(data->mlx));
	if (mlx_image_to_window(data->mlx, data->image, 0, 0) == -1)
		return (mlx_delete_image(data->mlx, data->image), \
		mlx_close_window(data->mlx), \
		display_error_message("Couldnt put image to window"));
	mlx_loop_hook(data->mlx, ft_hook_keys, data);
	mlx_loop_hook(data->mlx, change_image_size_hook, data);
	mlx_loop(data->mlx);
	mlx_terminate(data->mlx);
}
