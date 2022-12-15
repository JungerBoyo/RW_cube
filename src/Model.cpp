#include <Model.hpp>
#include <utils.hpp>

#include <fstream>
#include <vector>
#include <array>
#include <charconv>
#include <optional>
#include <unordered_map>

#include <glad/glad.h>
#include <fmt/format.h>
#include <lodepng.h>

using namespace rw_cube;

struct Face {
    std::array<std::uint32_t, 3> f0{{0, 0, 0}};
    std::array<std::uint32_t, 3> f1{{0, 0, 0}};
    std::array<std::uint32_t, 3> f2{{0, 0, 0}};
    std::optional<std::array<std::uint32_t, 3>> f3{ std::nullopt };
};

struct FaceElementHasher {
    std::size_t operator()(const std::array<std::uint32_t, 3>& key) const {
        std::size_t hash{ 0x0 };
        hash |= (static_cast<std::size_t>(key[0]) << 0);
        hash |= (static_cast<std::size_t>(key[1]) << 21); 
        hash |= (static_cast<std::size_t>(key[2]) << 42); 
        return hash;
    }
};

static auto parseFace(std::string_view str_face) {
    Face face{};
    
    // f0
    const auto f0_last_index = str_face.find_first_of(' ');
    auto result = std::from_chars(str_face.data(), &str_face[f0_last_index], face.f0[0]);
    result = std::from_chars(&result.ptr[1], &str_face[f0_last_index], face.f0[1]);
    result = std::from_chars(&result.ptr[1], &str_face[f0_last_index], face.f0[2]);
    // f1
    const auto f1_last_index = str_face.find_first_of(' ', f0_last_index + 1);
    result = std::from_chars(&str_face[f0_last_index + 1], &str_face[f1_last_index], face.f1[0]);
    result = std::from_chars(&result.ptr[1], &str_face[f1_last_index], face.f1[1]);
    result = std::from_chars(&result.ptr[1], &str_face[f1_last_index], face.f1[2]);
    // f2
    const auto f2_last_index = str_face.find_first_of(' ', f1_last_index + 1);
    if (f2_last_index != std::string::npos) {
        result = std::from_chars(&str_face[f1_last_index + 1], &str_face[f2_last_index], face.f2[0]);
        result = std::from_chars(&result.ptr[1], &str_face[f2_last_index], face.f2[1]);
        result = std::from_chars(&result.ptr[1], &str_face[f2_last_index], face.f2[2]);
        // f3 
        const auto f3_last_index = str_face.length();
        std::array<std::uint32_t, 3> face_f3{{0, 0, 0}};
        result = std::from_chars(&str_face[f2_last_index + 1], &str_face[f3_last_index], face_f3[0]);
        result = std::from_chars(&result.ptr[1], &str_face[f3_last_index], face_f3[1]);
        result = std::from_chars(&result.ptr[1], &str_face[f3_last_index], face_f3[2]);
        face.f3.emplace(face_f3);
    } else {
        result = std::from_chars(&str_face[f1_last_index + 1], &str_face[str_face.length()], face.f2[0]);
        result = std::from_chars(&result.ptr[1], &str_face[str_face.length()], face.f2[1]);
        result = std::from_chars(&result.ptr[1], &str_face[str_face.length()], face.f2[2]);
    }

    return face;
}

static auto parseVec3(std::string_view str_vec3) {
    const auto* last = &str_vec3[str_vec3.length()];
    float x{ 0.F };
    float y{ 0.F };
    float z{ 0.F };
    auto result = std::from_chars(
        str_vec3.data(),
        last,
        x
    );
    result = std::from_chars(
        &result.ptr[1],
        last,
        y
    );
    result = std::from_chars(
        &result.ptr[1],
        last,
        z
    );
    return std::make_tuple(x, y, z);
}
static auto parseVec2(std::string_view str_vec2) {
    const auto* last = &str_vec2[str_vec2.length()];
    float x{ 0.F };
    float y{ 0.F };
    auto result = std::from_chars(
        str_vec2.data(),
        last,
        x
    );
    result = std::from_chars(
        &result.ptr[1],
        last,
        y
    );
    return std::make_tuple(x, y);
}

static Model::Material loadMaterial(const std::filesystem::path& mtl_path) {
    std::ifstream stream(mtl_path);
    if (!stream.good()) {
		throw std::runtime_error(fmt::format(
			"failed to create stream from mtl file {}", mtl_path.string()
        ));
    }

    Model::Material result{};
    for(std::string line; std::getline(stream, line);) {
        switch(line[0]) {
        case 'N': {
            if (line[1] == 's') {
                std::from_chars(&line[3], &line[line.length() - 1], result.shininess);
            }
            break;
        }
        case 'K': {
            const auto[x, y, z] = parseVec3(&line[3]);
            switch(line[1]) {
            case 'a': {
                result.ambient[0] = x;
                result.ambient[1] = y;
                result.ambient[2] = z;
                break;
            }
            case 'd': {
                result.diffuse[0] = x;
                result.diffuse[1] = y;
                result.diffuse[2] = z;
                break;
            }
            case 's': {
                result.specular[0] = x;
                result.specular[1] = y;
                result.specular[2] = z;
                break;
            }
            }
            break;
        }
        case 'd': {
            std::from_chars(&line[2], &line[line.length() - 1], result.alpha);
            break;
        }
        }
    }
    return result;
}

Model::Model(bool is_spirv, const std::filesystem::path& obj_path, const std::filesystem::path& tex_path) :
    model_shader_(is_spirv, {
        is_spirv ? "shaders/bin/model_shader/vert.spv" : "shaders/src/model_shader/shader.vert",
        is_spirv ? "shaders/bin/model_shader/frag.spv" : "shaders/src/model_shader/shader.frag" 
    }){

    std::ifstream stream(obj_path.string());

    if (!stream.good()) {
		throw std::runtime_error(fmt::format(
			"failed to create stream from obj file {}", obj_path.string()
        ));
    }

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;

    using MapVerticesToIndices = 
        std::unordered_map<std::array<std::uint32_t, 3>, std::vector<std::uint32_t>, FaceElementHasher>;

    MapVerticesToIndices vertices_to_indices; 

    std::filesystem::path mtllib_path;
    mtllib_path.append("assets");
    mtllib_path.append("materials");

    std::string mtl_name;

    std::uint32_t num_indices{ 0 };

    bool gotline{ false };
    bool end{ false };
    for (std::string line; !end;) {
        if (!gotline) {
            if(!std::getline(stream, line)){
                break;
            }
        } else {
            gotline = false;
        }
        switch (line[0]) {
        case 'v': {
            switch (line[1]) {
            case ' ': {
                while(line[1] == ' ') {
                    const auto start_index = line.find_first_not_of(' ', 1);
                    const auto[x, y, z] = parseVec3({&line[start_index], line.length() - start_index});
                    positions.push_back(x);
                    positions.push_back(y);
                    positions.push_back(z);
                    if (!std::getline(stream, line)) {
                        end = true;
                        break;
                    }
                }
                gotline = true;
                break;
            }
            case 't': {
                while(line[1] == 't') {
                    const auto start_index = line.find_first_not_of(' ', 2);
                    const auto[u, v] = parseVec2({&line[start_index], line.length() - start_index});
                    texcoords.push_back(u);
                    texcoords.push_back(v);
                    if (!std::getline(stream, line)) {
                        end = true;
                        break;
                    }
                }
                gotline = true;
                break;
            }
            case 'n': {
                while(line[1] == 'n') {
                    const auto start_index = line.find_first_not_of(' ', 2);
                    const auto[x, y, z] = parseVec3({&line[start_index], line.length() - start_index});
                    normals.push_back(x);
                    normals.push_back(y);
                    normals.push_back(z);
                    if (!std::getline(stream, line)) {
                        end = true;
                        break;
                    }
                }
                gotline = true;
                break;
            }
            }
            break;
        }
        case 'f': {
            while(line[1] == ' ') {
                const auto start_index = line.find_first_not_of(' ', 1);
                const auto face = parseFace({&line[start_index], line.length() - start_index});

                const auto is_quad = face.f3.has_value();

                // f0
                if (vertices_to_indices.contains(face.f0)) {
                    auto& vertex_to_indices = vertices_to_indices[face.f0];
                    vertex_to_indices.push_back(num_indices);
                    if (is_quad) {
                        vertex_to_indices.push_back(num_indices + 5);
                    }
                } else {
                    vertices_to_indices.insert({
                        face.f0, is_quad ?
                            std::vector<std::uint32_t>{num_indices, num_indices + 5} :
                            std::vector<std::uint32_t>{num_indices}
                    });
                }

                // f1
                if (vertices_to_indices.contains(face.f1)) {
                    vertices_to_indices[face.f1].push_back(num_indices + 1);
                } else {
                    vertices_to_indices.insert({
                        face.f1, std::vector<std::uint32_t>{num_indices + 1}
                    });
                }

                // f2
                if (vertices_to_indices.contains(face.f2)) {
                    auto& vertex_to_indices = vertices_to_indices[face.f2];
                    vertex_to_indices.push_back(num_indices + 2);
                    if (is_quad) {
                        vertex_to_indices.push_back(num_indices + 3);
                    }
                } else {
                    vertices_to_indices.insert({
                        face.f2, is_quad ?
                            std::vector<std::uint32_t>{num_indices + 2, num_indices + 3} :
                            std::vector<std::uint32_t>{num_indices + 2}
                    });
                }

                // f3
                if (is_quad) {
                    if (vertices_to_indices.contains(face.f3.value())){
                        vertices_to_indices[face.f3.value()].push_back(num_indices + 4);
                    } else {
                        vertices_to_indices.insert({
                            face.f3.value(), std::vector<std::uint32_t>{num_indices + 4}
                        });
                    }
                }
                
                num_indices += is_quad ? 6 : 3;

                if (!std::getline(stream, line)) {
                    end = true;
                    break;
                }
            }
            gotline = true;
            break;
        } 
        default: {
            if (line.find("mtllib") != std::string::npos) {
                const auto pos = line.find_first_of(' ');
                mtllib_path.append(std::string(std::next(line.cbegin(), pos + 1), line.cend()));
            } else if (line.find("usemtl") != std::string::npos) {
                const auto pos = line.find_first_of(' ');
                mtl_name = std::string(std::next(line.cbegin(), pos + 1), line.cend());
            }
            break;
        }
        }
	}
    stream.close();

    indices_count_ = num_indices;

    std::vector<std::uint32_t> indices(num_indices);
    std::vector<float> vertices; 
    vertices.reserve(vertices_to_indices.size() * (3 + 2 + 3));

    // v
    // n
    // t

    std::uint32_t true_index{ 0 };
    for (const auto&[vertex, vertex_indices] : vertices_to_indices) {
        vertices.push_back(positions.at(3 * (vertex[0] - 1) + 0));
        vertices.push_back(positions.at(3 * (vertex[0] - 1) + 1));
        vertices.push_back(positions.at(3 * (vertex[0] - 1) + 2));

        vertices.push_back(texcoords.at(2 * (vertex[1] - 1) + 0));
        vertices.push_back(texcoords.at(2 * (vertex[1] - 1) + 1));

        vertices.push_back(normals.at(3 * (vertex[2] - 1) + 0));
        vertices.push_back(normals.at(3 * (vertex[2] - 1) + 1));
        vertices.push_back(normals.at(3 * (vertex[2] - 1) + 2));

        for(const auto index : vertex_indices) {
            indices[index] = true_index;
        }
        ++true_index;
    }

    std::array<std::uint32_t, 2> buffers{{0, 0}};
    glCreateBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());

    vbo_id_ = buffers[0];
    ebo_id_ = buffers[1];

    glNamedBufferStorage(
        vbo_id_, 
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        static_cast<const void*>(vertices.data()),
        0
    );
    glNamedBufferStorage(
        ebo_id_, 
        static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
        static_cast<const void*>(indices.data()),
        0
    );

    glCreateVertexArrays(1, &vao_id_);

    setVertexArrayLayout(
        vao_id_,
        vbo_id_,
        0U, 
        {
            {SHCONFIG_IN_POSITION_LOCATION, 3},
            {SHCONFIG_IN_TEXCOORD_LOCATION, 2},
            {SHCONFIG_IN_NORMAL_LOCATION, 3}
        }
    );

    glCreateTextures(GL_TEXTURE_2D, 1, &tex_id_);

    std::vector<unsigned char> img;
    std::uint32_t width{ 0 };
    std::uint32_t height{ 0 };

    lodepng::decode(img, width, height, tex_path.string());
    constexpr std::uint32_t channel_num{ 4 };

    const auto num_levels = 1 + static_cast<std::int32_t>(std::log2(std::max(
        static_cast<float>(width), 
        static_cast<float>(height)
    )));

    glTextureStorage2D(
        tex_id_, 
        num_levels,
        GL_RGBA8,
        static_cast<std::int32_t>(width), 
        static_cast<std::int32_t>(height)
    );

    glTextureParameteri(tex_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex_id_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTextureParameteri(tex_id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureSubImage2D(
        tex_id_,
        0,
        0, 0, 
        static_cast<std::int32_t>(width), 
        static_cast<std::int32_t>(height),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        static_cast<const void*>(img.data())
    );
    glGenerateTextureMipmap(tex_id_);
    glBindTextureUnit(SHCONFIG_2D_MODEL_TEX_BINDING, tex_id_);

    model_material_ = loadMaterial(mtllib_path);
}

void Model::draw() const {
    glDrawElements(GL_TRIANGLES, indices_count_, GL_UNSIGNED_INT, nullptr);
}

void Model::bind() const {
    model_shader_.bind();
    glBindVertexArray(vao_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id_);
}
void Model::deinit() {
    std::array<std::uint32_t, 2> buffers{{vbo_id_, ebo_id_}};
    glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
    vbo_id_ = 0;
    ebo_id_ = 0;

    glBindTextureUnit(SHCONFIG_2D_MODEL_TEX_BINDING, 0);
    glDeleteTextures(1, &tex_id_);
    tex_id_ = 0;

    glDeleteVertexArrays(1, &vao_id_);
    vao_id_ = 0;
}