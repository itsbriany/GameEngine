#pragma once
#include <boost/property_tree/ptree.hpp>
#include <b64/encode.h>
#include <b64/decode.h>
#include <unordered_set>
#include "tcp_client.h"

namespace GameEngine {
namespace Networking {

    class XMPPClient : public TCPClient {
        public:
            enum TransactionState {
                kSelectAuthenticationMechanism,
                kDecodeBase64Challenge
            };


            XMPPClient(boost::asio::io_service& io_service);
            XMPPClient(XMPPClient&& other);

            // Called each time a connection is established
            void OnConnect() final;

            // Called every time some bytes are read from the endpoint
            void OnRead(const std::string& bytes_read) final;

            // Called each time a boost::asio error occurs
            void OnError(const boost::system::error_code& ec) final;

            // Extract XML from the buffer
            std::stringstream ExtractXML(const std::string& start_tag, const std::string& end_tag);

            // Handles the Authentication Mechanism selection stage in SASL
            void HandleSelectAuthenticationMechanism(const std::string& bytes_read);

            // Extracts the authentication mechanism response from the buffer
            std::stringstream ExtractAuthenticationMechanismResponse(const std::string& bytes_read);

            // Parses the xml and searches for the available authentication
            // mechanisms
            std::unordered_set<std::string> ParseAuthenticationMechanisms(std::istream& xml_stream);

            // Handles the base 64 challenge decoding stage in SASL
            void HandleDecodeBase64Challenge(const std::string& bytes_read);

            std::string ParseBase64Challenge(std::istream& xml_stream);

            std::string DecodeBase64(const std::string& input);

            std::string& buffer() { return buffer_; }

            TransactionState state() const { return state_; }

            std::string log() override {
                return "[XMPPClient]";
            }

        private:
            void ParseAuthenticationMechanismsRecursive(const boost::property_tree::ptree& pt,
                                                        std::unordered_set<std::string>& authentication_mechanism_set);

            bool IsMD5AuthenticationMechanismAvailable(const std::unordered_set<std::string>& authentication_mechanism_set);
            std::ostringstream xml_version();

            // The first stream used to authenticate
            std::ostringstream initiation_stream();

            // Authentication mechanism stream
            std::ostringstream authentication_mechanism();

            // A base64 encoder
            base64::encoder base64_encoder_;

            // A base64 decoder
            base64::decoder base64_decoder_;

            // All data read from endpoint ends up here
            std::string buffer_;

            // The current transactional state
            TransactionState state_;
    };

}
}
