#pragma once


#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

enum class FSType
{
    CodeFile,
    JsonFile,
    Directory
};

class FileSystem
{
public:
    struct DirEntity;
    struct Entity
    {
        virtual ~Entity() = default;
        virtual FSType type() const = 0;

        virtual const std::string& code() const { throw std::runtime_error("entity is not a code entity"); }
        virtual DirEntity* as_dir() { throw std::runtime_error("entity is not a directory"); }
    };

    class CodeEntity: public Entity
    {
    public:
        virtual FSType type() const override;
        virtual const std::string& code() const override;

        std::string _code;
    };

    class DirEntity: public Entity
    {
    public:
        using entity_map = std::unordered_map<std::string, std::unique_ptr<Entity>>;
        virtual FSType type() const override;
        virtual DirEntity* as_dir() override;
        Entity* entity(const std::string& file, bool wantdir, bool createdir);
        Entity* new_file(std::string file);
        void erase(const std::string& file);
    private:
        entity_map::iterator entity(const std::string& file, bool wantdir);
        entity_map::iterator semvered_entity(const std::string& file, std::string::size_type verpos, bool wantdir);
    private:
        std::unordered_map<std::string, std::unique_ptr<Entity>> _content;
    };
public:
    FileSystem();
    ~FileSystem();

    bool exists(const std::string& cwd, const std::string& path);
    FSType type(const std::string& cwd, const std::string& path);

    Entity* write(const std::string& cwd, const std::string& path, std::string content);
    const std::string& read(const std::string& cwd, const std::string& path);
    bool erase(const std::string& cwd, const std::string& path);

private:
    enum class FSOperation
    {
        none,
        write,
        erase
    };
    Entity* entity_checked(const std::string& cwd, const std::string& file, FSOperation op = FSOperation::none);
    Entity* entity(const std::string& cwd, const std::string& file, FSOperation op = FSOperation::none);

private:
    DirEntity _root;
};