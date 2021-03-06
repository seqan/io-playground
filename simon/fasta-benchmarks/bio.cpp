#include <bio/seq_io/reader.hpp>

template <typename... Ts>
void noOpt(Ts&&...) {
    asm("");
}


void bio_bench(std::string const& _file) {

    std::filesystem::path fasta_file{_file};

    std::array<int, 5> ctChars{};

    auto fin  = bio::seq_io::reader{fasta_file};
    for (auto && [id, seq, qual] : fin) {
        for (auto c : seq) {
            ctChars[c.to_rank()] += 1;
        }
    }
    size_t a{};
    for (int i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}
/*int main(int argc, char** argv) {
    bio_bench(argv[1]);
}*/

