#pragma once

#include "reader.h"

#include <filesystem>
#include <functional>
#include <ostream>
#include <memory>
#include <variant>
#include <span>

namespace io3::vcf {

struct writer_config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

    // This is only relevant if a stream is being used
    bool compressed{};
};


struct writer_pimpl;

struct writer {
private:
    std::unique_ptr<writer_pimpl> pimpl;

public:
    writer(writer_config config);
    ~writer();
    void writeHeader(std::string_view key, std::string_view value);
    void addGenotype(std::string genotype);
    void write(record_view record);
};

}
