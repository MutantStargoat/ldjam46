#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "opengl.h"

class Image;

enum TextureType { TEX_2D, TEX_CUBE };

class Texture {
private:
	unsigned int id;
	unsigned int target;
	unsigned int texfmt;
	int sz[3];
	Image *img;
	static Image *default_img;

	Texture(const Texture &tex) {}
	Texture &operator =(const Texture &tex) { return *this; }

	void set_image_2d(const Image &img);
	bool set_image_cube(const Image &img, int idx);
	bool set_image_cube(const Image &img);

	/* for loading multiple cubemap faces from a single image */
	bool set_cube_multi(const Image &img, const float *xoffsets, const float *yoffsets, float sz,
		unsigned int rotmask = 0);

public:
	Texture();
	~Texture();

	void set_wrapping(unsigned int wrap);
	void set_filtering(unsigned int filt);
	void set_filtering(unsigned int min_filt, unsigned int mag_filt);

	unsigned int get_format() const;

	int get_size(int dim) const;
	int get_width() const;
	int get_height() const;

	void create(int xsz, int ysz, TextureType type = TEX_2D, unsigned int ifmt = GL_RGBA);
	void create_default(TextureType type = TEX_2D);
	void set_image(const Image &img, int idx = -1);

	bool load(const char *fname);
	bool load_cube(const char *fname);

	unsigned int get_id() const;
	TextureType get_type() const;

	void bind(int tex_unit = 0) const;
};

void bind_texture(Texture *tex, int tunit = 0);
int next_pow2(int x);

#endif	// TEXTURE_H_
