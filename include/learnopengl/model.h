#ifndef MODEL_H
#define MODEL_H

#include "learnopengl/mesh.h"

// #include <assimp/scene.h>
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiScene;
enum aiTextureType;

unsigned int TextureFromFile(const char *path, const std::string &directory);

class Model
{
public:
    std::vector<Mesh> meshes;
    // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Texture> textures_loaded;

    Model(char *path);
    void Draw(Shader &shader);

private:
    // model data
    std::string directory;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};

#endif