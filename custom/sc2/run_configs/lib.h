#pragma once
//
// Created by dewe on 7/14/22.
//
#include <utility>

#include "filesystem"
#include "optional"
#include "unordered_map"
#include "vector"
#include "fstream"
#include "argparse/argparse.hpp"


static argparse::ArgumentParser flags;

namespace sc2{

    struct Version{
        std::string game_version;
        int build_version{};
        std::optional<std::string> data_version, binary;

        Version()=default;
        Version(std::string  gV, int bV,
                std::optional<std::string>  dV,
                std::optional<std::string>  binary):game_version(std::move(gV)),
                                                          build_version(bV), data_version(std::move(dV)),
                                                          binary(std::move(binary)){}
        bool operator==(Version const& other) const= default;
        friend std::ostream& operator<<(std::ostream& os, Version const& v){
            os << "Version(game_version=" << v.game_version << ", build_version=" << v.build_version << ", data_version="
            << v.data_version.value_or("None") << ", binary=" << v.binary.value_or("None") << " );\n";
            return os;
        }
    };

    static  std::unordered_map<std::string, Version> version_dict( std::vector<Version> const& versions ){
        std::unordered_map<std::string, Version> dict;
        for(auto const& v: versions)
            dict.emplace(v.game_version, v);
        return dict;
    }


    const auto None = std::nullopt;

    const auto VERSIONS = version_dict({
                                               Version("3.13.0", 52910, "8D9FEF2E1CF7C6C9CBE4FBCA830DDE1C", None),
                                               Version("3.14.0", 53644, "CA275C4D6E213ED30F80BACCDFEDB1F5", None),
                                               Version("3.15.0", 54518, "BBF619CCDCC80905350F34C2AF0AB4F6", None),
                                               Version("3.15.1", 54518, "6EB25E687F8637457538F4B005950A5E", None),
                                               Version("3.16.0", 55505, "60718A7CA50D0DF42987A30CF87BCB80", None),
                                               Version("3.16.1", 55958, "5BD7C31B44525DAB46E64C4602A81DC2", None),
                                               Version("3.17.0", 56787, "DFD1F6607F2CF19CB4E1C996B2563D9B", None),
                                               Version("3.17.1", 56787, "3F2FCED08798D83B873B5543BEFA6C4B", None),
                                               Version("3.17.2", 56787, "C690FC543082D35EA0AAA876B8362BEA", None),
                                               Version("3.18.0", 57507, "1659EF34997DA3470FF84A14431E3A86", None),
                                               Version("3.19.0", 58400, "2B06AEE58017A7DF2A3D452D733F1019", None),
                                               Version("3.19.1", 58400, "D9B568472880CC4719D1B698C0D86984", None),
                                               Version("4.0.0", 59587, "9B4FD995C61664831192B7DA46F8C1A1", None),
                                               Version("4.0.2", 59587, "B43D9EE00A363DAFAD46914E3E4AF362", None),
                                               Version("4.1.0", 60196, "1B8ACAB0C663D5510941A9871B3E9FBE", None),
                                               Version("4.1.1", 60321, "5C021D8A549F4A776EE9E9C1748FFBBC", None),
                                               Version("4.1.2", 60321, "33D9FE28909573253B7FC352CE7AEA40", None),
                                               Version("4.1.3", 60321, "F486693E00B2CD305B39E0AB254623EB", None),
                                               Version("4.1.4", 60321, "2E2A3F6E0BAFE5AC659C4D39F13A938C", None),
                                               Version("4.2.0", 62347, "C0C0E9D37FCDBC437CE386C6BE2D1F93", None),
                                               Version("4.2.1", 62848, "29BBAC5AFF364B6101B661DB468E3A37", None),
                                               Version("4.2.2", 63454, "3CB54C86777E78557C984AB1CF3494A0", None),
                                               Version("4.2.3", 63454, "5E3A8B21E41B987E05EE4917AAD68C69", None),
                                               Version("4.2.4", 63454, "7C51BC7B0841EACD3535E6FA6FF2116B", None),
                                               Version("4.3.0", 64469, "C92B3E9683D5A59E08FC011F4BE167FF", None),
                                               Version("4.3.1", 65094, "E5A21037AA7A25C03AC441515F4E0644", None),
                                               Version("4.3.2", 65384, "B6D73C85DFB70F5D01DEABB2517BF11C", None),
                                               Version("4.4.0", 65895, "BF41339C22AE2EDEBEEADC8C75028F7D", None),
                                               Version("4.4.1", 66668, "C094081D274A39219061182DBFD7840F", None),
                                               Version("4.5.0", 67188, "2ACF84A7ECBB536F51FC3F734EC3019F", None),
                                               Version("4.5.1", 67188, "6D239173B8712461E6A7C644A5539369", None),
                                               Version("4.6.0", 67926, "7DE59231CBF06F1ECE9A25A27964D4AE", None),
                                               Version("4.6.1", 67926, "BEA99B4A8E7B41E62ADC06D194801BAB", None),
                                               Version("4.6.2", 69232, "B3E14058F1083913B80C20993AC965DB", None),
                                               Version("4.7.0", 70154, "8E216E34BC61ABDE16A59A672ACB0F3B", None),
                                               Version("4.7.1", 70154, "94596A85191583AD2EBFAE28C5D532DB", None),
                                               Version("4.8.0", 71061, "760581629FC458A1937A05ED8388725B", None),
                                               Version("4.8.1", 71523, "FCAF3F050B7C0CC7ADCF551B61B9B91E", None),
                                               Version("4.8.2", 71663, "FE90C92716FC6F8F04B74268EC369FA5", None),
                                               Version("4.8.3", 72282, "0F14399BBD0BA528355FF4A8211F845B", None),
                                               Version("4.8.4", 73286, "CD040C0675FD986ED37A4CA3C88C8EB5", None),
                                               Version("4.8.5", 73559, "B2465E73AED597C74D0844112D582595", None),
                                               Version("4.8.6", 73620, "AA18FEAD6573C79EF707DF44ABF1BE61", None),
                                               Version("4.9.0", 74071, "70C74A2DCA8A0D8E7AE8647CAC68ACCA", None),
                                               Version("4.9.1", 74456, "218CB2271D4E2FA083470D30B1A05F02", None),
                                               Version("4.9.2", 74741, "614480EF79264B5BD084E57F912172FF", None),
                                               Version("4.9.3", 75025, "C305368C63621480462F8F516FB64374", None),
                                               Version("4.10.0", 75689, "B89B5D6FA7CBF6452E721311BFBC6CB2", None),
                                               Version("4.10.1", 75800, "DDFFF9EC4A171459A4F371C6CC189554", None),
                                               Version("4.10.2", 76052, "D0F1A68AA88BA90369A84CD1439AA1C3", None),
                                               Version("4.10.3", 76114, "CDB276D311F707C29BA664B7754A7293", None),
                                               Version("4.10.4", 76811, "FF9FA4EACEC5F06DEB27BD297D73ED67", None),
                                               Version("4.11.0", 77379, "70E774E722A58287EF37D487605CD384", None),
                                               Version("4.11.1", 77379, "F92D1127A291722120AC816F09B2E583", None),
                                               Version("4.11.2", 77535, "FC43E0897FCC93E4632AC57CBC5A2137", None),
                                               Version("4.11.3", 77661, "A15B8E4247434B020086354F39856C51", None),
                                               Version("4.11.4", 78285, "69493AFAB5C7B45DDB2F3442FD60F0CF", None),
                                               Version("4.12.0", 79998, "B47567DEE5DC23373BFF57194538DFD3", None),
                                               Version("4.12.1", 80188, "44DED5AED024D23177C742FC227C615A", None),
                                               Version("5.0.0", 80949, None, None),  // Unknown data_version
                                               Version("5.0.1", 81009, "44DED5AED024D23177C742FC227C615A", None),
                                               Version("5.0.2", 81102, "5FD8D4B6B52723B44862DF29F232CF31", None),
                                               Version("5.0.3", 81433, "5FD8D4B6B52723B44862DF29F232CF31", None),
                                               Version("5.0.4", 82457, "D2707E265785612D12B381AF6ED9DBF4", None),
                                               Version("5.0.5", 82893, "D795328C01B8A711947CC62AA9750445", None),
                                               Version("5.0.6", 83830, "B4745D6A4F982A3143C183D8ACB6C3E3", None),
                                               Version("5.0.7", 84643, "A389D1F7DF9DD792FBE980533B7119FF", None),
                                               Version("5.0.8", 86383, "22EAC562CD0C6A31FB2C2C21E3AA3680", None),
                                               Version("5.0.9", 87702, "F799E093428D419FD634CCE9B925218C", None),
                                       });

    class RunConfig {

        inline static std::vector< std::array<std::string, 2> > KNOWN_GL_LIBS{
                {"-eglpath", "libEGL.so"},
                {"-eglpath", "libEGL.so.1"},
                {"-osmesapath", "libOSMesa.so"},
                {"-osmesapath", "libOSMesa.so.8"},  // Ubuntu 16.04
                {"-osmesapath", "libOSMesa.so.6"},  // Ubuntu 14.04
        };

    public:
        struct Option {
            std::filesystem::path replay_dir, data_dir;
            std::optional<std::filesystem::path> tmp_dir, cwd;
            std::string version;
        };

        explicit RunConfig(std::string _version);

        struct std::unique_ptr<class StarcraftProcess>  start(
                bool want_rgb=true, std::vector<unsigned short> const& extra_ports={},
                    std::vector<std::string> extra_args={});

        inline auto tmpDir() const { return opt.tmp_dir; }
        inline auto dataDir() const { return opt.data_dir; }

        std::string map_data(std::string const& map_name, std::optional<int> const& players);

    private:
        Option opt;
        Version version;
        std::string m_execName;

        [[nodiscard]] virtual inline std::string name() const { return "Linux"; }

        [[nodiscard]] static inline RunConfig make(Version const& v) {
            return RunConfig(v.game_version);
        }

        [[nodiscard]] static inline RunConfig make(std::optional<std::string> const& v) {
            return RunConfig(v.value_or(""));
        }

        std::unordered_map<std::basic_string<char>, Version>
        get_versions(std::optional<std::string> const &containing = std::nullopt);

        [[nodiscard]] Version get_version(std::string game_version);

        Version get_version(Version const &game_version);
    };

}
