#include <seqan3/io/sequence_file/all.hpp>

void seqan3_bench(std::string const& _file) {
    std::filesystem::path fasta_file{_file};
    auto reader = seqan3::sequence_file_input{fasta_file};

    std::array<int, 5> ctChars{};
    //!TODO this should really be [id, seq, qual], seqan3 is backwards
    for (auto && [seq, id, qual] : reader) {
        for (auto c : seq) {
            ctChars[c.to_rank()] += 1;
        }
    }

    size_t a{};
    for (size_t i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}
