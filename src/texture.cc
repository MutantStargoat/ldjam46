#include <math.h>
#include <assert.h>
#include "texture.h"
#include "image.h"
#include "opengl.h"
#include "imago2.h"

static int glifmt_from_ifmt(unsigned int ifmt);
static int glfmt_from_ifmt(unsigned int ifmt);
static int gltype_from_ifmt(unsigned int ifmt);

static int glifmt_from_imgfmt(Image::Format fmt);

static unsigned int type_to_target(TextureType type);
static TextureType target_to_type(unsigned int targ);

static unsigned int cur_target[8] = {
	GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D,
	GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D, GL_TEXTURE_2D
};

static unsigned int cube_faces[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};


void bind_texture(Texture *tex, int tunit)
{
	if(tex) {
		tex->bind(tunit);
	} else {
		glActiveTexture(GL_TEXTURE0 + tunit);
		glBindTexture(cur_target[tunit], 0);
		assert(glGetError() == GL_NO_ERROR);
		glActiveTexture(GL_TEXTURE0);
	}
}

int next_pow2(int x)
{
	x--;
	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;
	return x + 1;
}


Image *Texture::default_img;

Texture::Texture()
{
	target = 0;
	sz[0] = sz[1] = sz[2] = 0;
	texfmt = 0;

	img = 0;
	glGenTextures(1, &id);
}

Texture::~Texture()
{
	if(id) {
		glDeleteTextures(1, &id);
	}
	if(img) {
		delete img;
	}
}

void Texture::set_wrapping(unsigned int wrap)
{
	if(!target) {
		return;
	}

	glBindTexture(target, id);
	assert(glGetError() == GL_NO_ERROR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);
}

void Texture::set_filtering(unsigned int filt)
{
	unsigned int mag_filter;

	if(!target) {
		return;
	}

	switch(filt) {
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		mag_filter = GL_LINEAR;
		break;

	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		mag_filter = GL_NEAREST;
		break;

	default:
		mag_filter = filt;
	}

	set_filtering(filt, mag_filter);
}

void Texture::set_filtering(unsigned int min_filt, unsigned int mag_filt)
{
	glBindTexture(target, id);
	assert(glGetError() == GL_NO_ERROR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filt);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filt);
}

unsigned int Texture::get_format() const
{
	return texfmt;
}

int Texture::get_size(int dim) const
{
	if(dim < 0 || dim >= 3) {
		return 0;
	}
	return sz[dim];
}

int Texture::get_width() const
{
	return sz[0];
}

int Texture::get_height() const
{
	return sz[1];
}

unsigned int Texture::get_id() const
{
	return id;
}

TextureType Texture::get_type() const
{
	return target_to_type(target);
}

void Texture::bind(int tex_unit) const
{
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(target, id);
	assert(glGetError() == GL_NO_ERROR);
	glActiveTexture(GL_TEXTURE0);

	cur_target[tex_unit] = target;
}


void Texture::create(int xsz, int ysz, TextureType textype, unsigned int ifmt)
{
	if(textype == TEX_CUBE && xsz != ysz) {
		fprintf(stderr, "trying to create cubemap with different width and height (%dx%d)\n", xsz, ysz);
		return;
	}

	int fmt = glfmt_from_ifmt(ifmt);
	int type = gltype_from_ifmt(ifmt);

	target = type_to_target(textype);

	glBindTexture(target, id);
	assert(glGetError() == GL_NO_ERROR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch(textype) {
	case TEX_2D:
		glTexImage2D(GL_TEXTURE_2D, 0, glifmt_from_ifmt(ifmt), xsz, ysz, 0, fmt, type, 0);
		break;

	case TEX_CUBE:
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for(int i=0; i<6; i++) {
			glTexImage2D(cube_faces[i], 0, ifmt, xsz, ysz, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		}
		break;
	}

	sz[0] = xsz;
	sz[1] = ysz;
	texfmt = ifmt;
}

#define DEF_IMAGE_SIZE	64
void Texture::create_default(TextureType type)
{
	if(!default_img) {
		default_img = new Image;
		default_img->create(DEF_IMAGE_SIZE, DEF_IMAGE_SIZE, Image::FMT_RGBA);

		unsigned char *pixels = (unsigned char*)default_img->get_pixels();
		for(int i=0; i<DEF_IMAGE_SIZE; i++) {
			for(int j=0; j<DEF_IMAGE_SIZE; j++) {
				bool chess = ((i >> 3) & 1) == ((j >> 3) & 1);
				pixels[0] = chess ? 255 : 32;
				pixels[1] = 64;
				pixels[2] = chess ? 32 : 255;
				pixels[3] = 255;
				pixels += 4;
			}
		}
	}

	switch(type) {
	case TEX_2D:
		set_image(*default_img);
		break;

	case TEX_CUBE:
		for(int i=0; i<6; i++) {
			set_image(*default_img, i);
		}
		break;
	}
}

void Texture::set_image(const Image &img, int idx)
{
	if(idx >= 0 && idx < 6) {
		set_image_cube(img, idx);
	} else {
		if(!set_image_cube(img)) {
			set_image_2d(img);
		}
	}
}

void Texture::set_image_2d(const Image &img)
{
	texfmt = glifmt_from_imgfmt(img.get_format());
	unsigned int fmt = glfmt_from_ifmt(texfmt);
	unsigned int type = gltype_from_ifmt(texfmt);

	sz[0] = img.get_width();
	sz[1] = img.get_height();

	target = GL_TEXTURE_2D;
	glBindTexture(target, id);
	assert(glGetError() == GL_NO_ERROR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef __GLEW_H__
	if(GLEW_SGIS_generate_mipmap) {
		glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
#endif
		glTexImage2D(target, 0, texfmt, sz[0], sz[1], 0, fmt, type, img.get_pixels());
#ifdef __GLEW_H__
	} else {
		gluBuild2DMipmaps(target, texfmt, sz[0], sz[1], fmt, type, img.get_pixels());
	}
#endif

#ifdef GL_ES_VERSION_2_0
	glGenerateMipmap(target);
#endif
}

bool Texture::set_image_cube(const Image &img, int idx)
{
	unsigned int err;
	if(idx < 0 || idx >= 6) {
		return false;
	}

	texfmt = glifmt_from_imgfmt(img.get_format());
	unsigned int fmt = glfmt_from_ifmt(texfmt);
	unsigned int type = gltype_from_ifmt(texfmt);

	sz[0] = img.get_width();
	sz[1] = img.get_height();

	target = GL_TEXTURE_CUBE_MAP;
	glBindTexture(target, id);
	if((err = glGetError()) == GL_INVALID_OPERATION) {
		return false;	// probably not a cubemap
	}
	assert(err == GL_NO_ERROR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(cube_faces[idx], 0, texfmt, sz[0], sz[1], 0, fmt, type, img.get_pixels());
	return true;
}

bool Texture::set_image_cube(const Image &img)
{
	static const float one_third = 1.0 / 3.0;
	static const float two_thirds = 2.0 / 3.0;
	static const float hcross[2][6] = {
		{0.5, 0.0, 0.25, 0.25, 0.25, 0.75}, {one_third, one_third, 0.0, two_thirds, one_third, one_third} };
	static const float vcross[2][6] = {
		{two_thirds, 0.0, one_third, one_third, one_third, one_third}, {0.25, 0.25, 0.0, 0.5, 0.25, 0.75} };
	static const float hsix[2][6] = {
		{0.0, 0.0, one_third, one_third, two_thirds, two_thirds}, {0.0, 0.5, 0.0, 0.5, 0.0, 0.5} };

	int xsz = img.get_width();
	int ysz = img.get_height();

	if((xsz << 8) / 4 == (ysz << 8) / 3) {
		// horizontal cross, assume the vertical bit is center-left
		return set_cube_multi(img, hcross[0], hcross[1], xsz / 4);
	}
	if((xsz << 8) / 3 == (ysz << 8) / 4) {
		// vertical cross, assume the horizontal bit is center-top (180-rotated image 5)
		return set_cube_multi(img, vcross[0], vcross[1], ysz / 4, (1 << 5));
	}
	if((xsz << 8) / 3 == (ysz << 8) / 2) {
		// horizontal sixpack
		return set_cube_multi(img, hsix[0], hsix[1], ysz / 2);
	}

	return false;
}


bool Texture::load(const char *fname)
{
	Image img;
	if(!img.load(fname)) {
		fprintf(stderr, "failed to load 2D texture: %s\n", fname);
		return false;
	}
	set_image(img);

	printf("loaded 2D texture: %s\n", fname);
	return true;
}

bool Texture::load_cube(const char *fname)
{
	Image img;
	if(!img.load(fname)) {
		return false;
	}
	return set_image_cube(img);
}

bool Texture::set_cube_multi(const Image &img, const float *xoffsets, const float *yoffsets, float sz,
		unsigned int rotmask)
{
	for(int i=0; i<6; i++) {
		Image face;

		int xoffs = xoffsets[i] * img.get_width();
		int yoffs = yoffsets[i] * img.get_height();

		if(!face.set_pixels(sz, sz, img.get_pixels(), xoffs, yoffs, img.get_width(), img.get_format())) {
			return false;
		}

		if(rotmask & (1 << i)) {
			face.rotate_180();
		}
		set_image_cube(face, i);
	}
	return true;
}

static int glifmt_from_ifmt(unsigned int ifmt)
{
#ifdef GL_ES_VERSION_2_0
	switch(ifmt) {
	case GL_LUMINANCE16F_ARB:
	case GL_LUMINANCE32F_ARB:
		ifmt = GL_LUMINANCE;
		break;

	case GL_RGB16F:
	case GL_RGB32F:
		ifmt = GL_RGB;
		break;

	case GL_RGBA16F:
	case GL_RGBA32F:
		ifmt = GL_RGBA;
		break;

	default:
		break;
	}
#endif
	return ifmt;	// by default just pass it through...
}

static int glfmt_from_ifmt(unsigned int ifmt)
{
	switch(ifmt) {
	case GL_LUMINANCE16F_ARB:
	case GL_LUMINANCE32F_ARB:
	case GL_SLUMINANCE:
		return GL_LUMINANCE;

	case GL_RGB16F:
	case GL_RGB32F:
	case GL_SRGB:
		return GL_RGB;

	case GL_RGBA16F:
	case GL_RGBA32F:
	case GL_SRGB_ALPHA:
		return GL_RGBA;

	default:
		break;
	}
	return ifmt;
}

static int gltype_from_ifmt(unsigned int ifmt)
{
	switch(ifmt) {
	case GL_RGB16F:
	case GL_RGBA16F:
	case GL_LUMINANCE16F_ARB:
#ifdef GL_ES_VERSION_2_0
		return GL_HALF_FLOAT_OES;
#endif
	case GL_RGB32F:
	case GL_RGBA32F:
	case GL_LUMINANCE32F_ARB:
		return GL_FLOAT;

	default:
		break;
	}
	return GL_UNSIGNED_BYTE;
}

static int glifmt_from_imgfmt(Image::Format fmt)
{
	switch(fmt) {
	case Image::FMT_GREY:
		return GL_SLUMINANCE;
	case Image::FMT_GREY_FLOAT:
		return GL_LUMINANCE16F_ARB;
	case Image::FMT_RGB:
		return GL_SRGB;
	case Image::FMT_RGB_FLOAT:
		return GL_RGB16F;
	case Image::FMT_RGBA:
		return GL_SRGB_ALPHA;
	case Image::FMT_RGBA_FLOAT:
		return GL_RGBA16F;
	default:
		break;
	}
	return 0;
}

static unsigned int type_to_target(TextureType type)
{
	return type == TEX_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
}

static TextureType target_to_type(unsigned int targ)
{
	return targ == GL_TEXTURE_CUBE_MAP ? TEX_CUBE : TEX_2D;
}
