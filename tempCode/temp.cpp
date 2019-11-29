#include <ndn-cxx/face.hpp>

#include "ndn-cxx/util/io.hpp"
#include "ndn-cxx/security/transform/base64-encode.hpp"
#include "ndn-cxx/security/transform/buffer-source.hpp"
#include "ndn-cxx/security/transform/public-key.hpp"
#include "ndn-cxx/security/transform/stream-sink.hpp"

#include <iostream>
#include <string>
#include <fstream>

using namespace ndn;


int
main(int argc, char** argv)
{
  const std::string requestFile = "../key/11144482685046850104.key";
  security::v2::Certificate certRequest;
  shared_ptr<security::v2::Certificate> cert;
  cert = io::load<security::v2::Certificate>(requestFile);
  certRequest = *cert;
  Buffer keyContent = certRequest.getPublicKey();  
  for(auto it : keyContent){
  	std::cout << it;
  }
  std::cout << certRequest.getKeyName() << std::endl;
  //std::cout << keyContent.data() << std::endl;
  security::transform::PublicKey pubKey;
  pubKey.loadPkcs8(keyContent.data(), keyContent.size());
  return 0;
}
