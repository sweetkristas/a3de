#pragma once

#include <map>
#include <string>

#include <GLES2/gl2.h>
#include "ref_counted_ptr.hpp"

namespace shader
{
	// Abstraction of vertex/geometry/fragment shader
	class shader
	{
	public:
		shader() : type_(0), shader_(0)
		{}
		explicit shader(GLenum type, const std::string& name, const std::string& code);
		GLuint get() const { return shader_; }
		std::string name() const { return name_; }
	protected:
		bool compile(const std::string& code);
	private:
		GLenum type_;
		GLuint shader_;
		std::string name_;
	};

	struct actives
	{
		// Name of variable.
		std::string name;
		// type of the uniform/attribute variable
		GLenum type;
		// If an array type, this is the maximum number of array elements used 
		// in the program. Value is 1 if type is not an array type.
		GLsizei num_elements;
		// Location of the active uniform/attribute
		GLint location;
	};

	class program_object : public reference_counted_ptr
	{
	public:
		program_object();
		explicit program_object(const std::string& name, const shader& vs, const shader& fs);
		virtual ~program_object()
		{}
		void init(const std::string& name, const shader& vs, const shader& fs);
		std::string name() const { return name_; }
		GLuint get_attribute(const std::string& attr) const;
		GLuint get_uniform(const std::string& attr) const;
	protected:
		bool link();
		bool queryUniforms();
		bool queryAttributes();

		std::vector<GLint> active_attributes_;
	private:
		std::string name_;
		shader vs_;
		shader fs_;
		GLuint object_;
		std::map<std::string, actives> attribs_;
		std::map<std::string, actives> uniforms_;
	};

	typedef boost::intrusive_ptr<program_object> program_object_ptr;
	typedef boost::intrusive_ptr<const program_object> const_program_object_ptr;
}
