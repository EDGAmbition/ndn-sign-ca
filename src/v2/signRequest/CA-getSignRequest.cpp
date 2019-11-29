#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/time.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <functional>
#include <string>

#include "ndn-cxx/security/transform/base64-encode.hpp"
#include "ndn-cxx/security/transform/buffer-source.hpp"
#include "ndn-cxx/security/transform/public-key.hpp"
#include "ndn-cxx/security/transform/stream-sink.hpp"

#include "ndn-cxx/security/v2/additional-description.hpp"
#include "ndn-cxx/util/io.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>


// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespaces should be used to prevent/limit name conflicts

class Consumer
{
public:

  Consumer(const std::string& prefix){
    m_prefix = prefix + "/getSignRequest";   
  }
  void
  run()
  {
    Name interestName(m_prefix);
    //interestName.appendVersion();

    Interest interest(interestName);
    interest.setCanBePrefix(false);
    interest.setMustBeFresh(true);
    interest.setInterestLifetime(6_s); // The default is 4 seconds

    std::cout << "Sending Interest " << interest << std::endl;
    m_face.expressInterest(interest,
                           bind(&Consumer::onData, this,  _1, _2),
                           bind(&Consumer::onNack, this, _1, _2),
                           bind(&Consumer::onTimeout, this, _1));

    // processEvents will block until the requested data is received or a timeout occurs
    m_face.processEvents();
  }

private:
	std::string
  getKeyPrefix(const Interest& interest,const std::string caPrefix)
  {
    std::string subName = interest.getName().getSubName(3).toUri();
    std::string keyPrefix = caPrefix + subName;
    return keyPrefix;
  }
  
	char *
  fileLocation(const Interest& interest,std::string dir,std::string extension)
  {
    // cert file location
    
    std::string keyPrefix = getKeyPrefix(interest,"/ndn/ca"); 
    //std::cout << keyPrefix << std::endl; 
    std::string keyPrefix_hash = std::to_string(std::hash<std::string>{}(keyPrefix));      
    std::string location = dir + keyPrefix_hash + extension;
    
    char * location_c = new char [location.length() + 1];
    strcpy(location_c,location.c_str());
    
    return location_c;
  }
  void 
  writeFile(std::string loc,std::string content)
  {
    std::ofstream out(loc,std::ios::out | std::ios::trunc);
    out << content;
    out.close();
  }
  void
  onData(const Interest&, const Data& data) 
  {
  
  	char * content_c = new char[data.getContent().value_size() + 1];
    memcpy(content_c,data.getContent().value(),data.getContent().value_size());
    std::string content = content_c;
    //std::cout << content << std::endl;
    
    //std::string loc = fileLocation(Interest,"./",".req");
    //std::cout << loc << std::endl;
    std::string loc = "./2.req";
    writeFile(loc,content);
  	
  	Name signId("/ndn/ca/1");
  	std::string issuerId = "BCCA";
  	// vaild period
    time::system_clock::TimePoint notBefore;
 	  time::system_clock::TimePoint notAfter;
 	  notBefore = time::system_clock::now();
 	  notAfter = notBefore + 1_days * 7;
 	  
 	  security::v2::KeyChain keyChain;
  	security::v2::Certificate certRequest;
  	
  	certRequest = *io::load<security::v2::Certificate>("./2.req");
  	
  	Buffer keyContent = certRequest.getPublicKey();
    security::transform::PublicKey pubKey;
    pubKey.loadPkcs8(keyContent.data(), keyContent.size());
    
    Name certName = certRequest.getKeyName();
  	certName.append(issuerId).appendVersion();
  	
  	security::v2::Certificate cert;
  	cert.setName(certName);
 	  cert.setContent(certRequest.getContent());
 	  // TODO: add ability to customize
    cert.setFreshnessPeriod(1_h);

    SignatureInfo signatureInfo;
  	signatureInfo.setValidityPeriod(security::ValidityPeriod(notBefore, notAfter));
  	
  	security::Identity identity;
  	identity = keyChain.getPib().getIdentity(signId);
  	keyChain.sign(cert, security::SigningInfo(identity).setSignatureInfo(signatureInfo));
  	const Block& wire = cert.wireEncode();
 	  {
    	using namespace security::transform;
    	std::ofstream os("signed-cert.cert");
   	  bufferSource(wire.wire(), wire.size()) >> base64Encode(true) >> streamSink(os);
    }
  }

  void
  onNack(const Interest&, const lp::Nack& nack) const
  {
    std::cout << "Received Nack with reason " << nack.getReason() << std::endl;
  }

  void
  onTimeout(const Interest& interest) const
  {
    std::cout << "Timeout for " << interest << std::endl;
  }

private:
  Face m_face;
  std::string m_prefix;
};

} // namespace ndn

int
main(int argc, char** argv)
{
  try {
    ndn::Consumer consumer(argv[1]);
    consumer.run();
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
