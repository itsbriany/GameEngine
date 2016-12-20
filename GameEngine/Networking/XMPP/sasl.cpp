#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "sasl.h"

namespace GameEngine {
namespace Networking {

std::unordered_map<SASL::Mechanism, std::string> SASL::s_digest_map = boost::assign::map_list_of
(kSHA1, "SCRAM-SHA-1")
(kMD5, "DIGEST-MD5")
(kNone, "PLAIN");

SASL::SASL(XMPPClient& client): XMPPState(client), m_transaction_state(kInitializeStream) { }

void SASL::Update() {
    switch (m_transaction_state) {
        case TransactionState::kInitializeStream:
            HandleInitializeStream();
            break;
        case TransactionState::kSelectAuthenticationMechanism:
            HandleSelectAuthenticationMechanism();
            break;
        case TransactionState::kDecodeBase64Challenge:
            HandleDecodeBase64Challenge();
            break;
        default:
            Logging::error("Unknown Transaction State", *this);
            m_client.Disconnect();
            return;
    }
}

void SASL::HandleInitializeStream() {
    m_client.Write(initiation_stream());
    m_transaction_state = TransactionState::kSelectAuthenticationMechanism;
}

void SASL::HandleSelectAuthenticationMechanism() {
    std::stringstream authentication_mechanism_response = ExtractAuthenticationMechanismResponse();
    if (authentication_mechanism_response.str().empty())
        return;

    std::unordered_set<std::string> authentication_mechanism_set = ParseAuthenticationMechanisms(authentication_mechanism_response);
    Mechanism mechanism = SetAuthenticationMechanism(authentication_mechanism_set);
    if (mechanism == kNone) {
        Logging::error("Only SRCAM-SHA1 and MD5 SASL authentication mechanism is supported", *this);
        m_client.Disconnect();
    }

    m_transaction_state = kDecodeBase64Challenge;
    Logging::debug("Initiating authentication exchange...", *this);
    m_client.Write(InitiateAuthenticationStream(mechanism));
}

SASL::Mechanism SASL::SetAuthenticationMechanism(const std::unordered_set<std::string>& authentication_mechanism_set) {
    if (IsSHA1AuthenticationMechanismAvailable(authentication_mechanism_set)) {
        return kSHA1;
    }
    if (IsMD5AuthenticationMechanismAvailable(authentication_mechanism_set)) {
        return kMD5;
    }
    return kNone;
}

void SASL::HandleDecodeBase64Challenge() {
    std::string start_tag = "<challenge";
    std::string end_tag = "</challenge>";
    std::stringstream extracted_response = ExtractXML(start_tag, end_tag);
    if (extracted_response.str().empty()) {
        return;
    }
    std::string base64_challenge = ParseBase64Challenge(extracted_response);
    std::string decoded_challenge = DecodeBase64(base64_challenge);

}

std::string SASL::ParseBase64Challenge(std::istream& xml_stream) const {
    boost::property_tree::ptree pt;
    read_xml(xml_stream, pt);
    boost::property_tree::ptree::const_iterator it = pt.begin();
    return it->second.get_value<std::string>();
}

std::string SASL::EncodeBase64(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;
    m_base64_encoder.encode(iss, oss);
    std::string encoded_without_linefeed = oss.str();
    boost::erase_all(encoded_without_linefeed, std::string("\n"));
    return encoded_without_linefeed;
}

std::string SASL::DecodeBase64(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;
    m_base64_decoder.decode(iss, oss);
    return oss.str();
}

std::string SASL::GenerateNonce() {
    std::string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    std::string nonce;
    for (int i = 0; i < 32; ++i) {
        nonce += chars[index_dist(rng)];
    }
    return nonce;
}

bool SASL::IsMD5AuthenticationMechanismAvailable(const std::unordered_set<std::string>& authentication_mechanism_set) {
    std::string md5 = "DIGEST-MD5";
    return authentication_mechanism_set.find(md5) != authentication_mechanism_set.end();
}

bool SASL::IsSHA1AuthenticationMechanismAvailable(const std::unordered_set<std::string>& authentication_mechanism_set) {
    std::string sha1 = "SCRAM-SHA-1";
    return authentication_mechanism_set.find(sha1) != authentication_mechanism_set.end();
}

std::stringstream SASL::ExtractXML(const std::string& start_tag, const std::string& end_tag) const {
    std::stringstream extracted_response;
    size_t found_start = m_client.buffer().find(start_tag);
    if (found_start == std::string::npos) {
        return extracted_response;
    }

    size_t found_end = m_client.buffer().find(end_tag);
    if (found_end == std::string::npos) {
        return extracted_response;
    }

    size_t bytes_to_extract = found_end + end_tag.size() - found_start;
    extracted_response << m_client.buffer().substr(found_start, bytes_to_extract);
    m_client.buffer() = m_client.buffer().substr(found_end + end_tag.size());
    return extracted_response;
}

std::stringstream SASL::ExtractAuthenticationMechanismResponse() const {
    std::string search_start = "<stream:features>";
    std::string search_end = "</stream:features>";
    return ExtractXML(search_start, search_end);
}

std::unordered_set<std::string> SASL::ParseAuthenticationMechanisms(std::istream& xml_stream) const {
    boost::property_tree::ptree pt;
    std::unordered_set<std::string> authentication_mechanism_set;
    read_xml(xml_stream, pt);
    ParseAuthenticationMechanismsRecursive(pt, authentication_mechanism_set);
    return authentication_mechanism_set;
}

void SASL::ParseAuthenticationMechanismsRecursive(const boost::property_tree::ptree& pt,
                                                        std::unordered_set<std::string>& authentication_mechanism_set) {
    for (boost::property_tree::ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
        if (it->first == "mechanism") {
            std::string mechanism_value = it->second.get_value<std::string>();
            authentication_mechanism_set.insert(mechanism_value);
        }
        ParseAuthenticationMechanismsRecursive(it->second, authentication_mechanism_set);
    }
}

std::ostringstream SASL::xml_version() {
    std::ostringstream oss;
    oss << "<?xml version='1.0'?>";
    return oss;
}

std::ostringstream SASL::initiation_stream() const {
    std::ostringstream oss;
    oss << xml_version().str();
    oss << "<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' to='" << m_client.host() << "' version='1.0'>";
    return oss;
}

std::string SASL::SHA1AuthenticationPayload() {
    std::ostringstream payload_stream;
    std::string g2s_header = "n,,";
    std::string nonce = GenerateNonce();
    payload_stream << g2s_header << "n=" << m_client.username() << ",r=" << nonce;
    return EncodeBase64(payload_stream.str());
}

std::ostringstream SASL::InitiateAuthenticationStream(Mechanism mechanism) {
    std::ostringstream authentication_stream;
    authentication_stream << "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='" << s_digest_map[mechanism] << '\'';
    if (mechanism == kSHA1) {
        authentication_stream << '>';
        authentication_stream << SHA1AuthenticationPayload();
        authentication_stream << "</auth>";
        return authentication_stream;
    }

    authentication_stream << "/>";
    return authentication_stream;
}

}
}
