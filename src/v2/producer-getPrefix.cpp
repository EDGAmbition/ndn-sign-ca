#include <ndn-cxx/face.hpp>

#include "ndn-cxx/security/key-chain.hpp"
#include "ndn-cxx/security/v2/additional-description.hpp"
#include "ndn-cxx/util/io.hpp"


#include <iostream>
#include <string>
#include <fstream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespaces should be used to prevent/limit name conflicts
namespace examples {

class Producer
{
public:
  Producer(const std::string &prefix) 
  {
    const std::string caPrefix_getPrefix = "/ndn/ca/getPrefix";
    
    m_prefix = caPrefix_getPrefix + prefix;
    short_prefix = prefix;
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
    std::cout << "function getName():"<< interest.getName() << std::endl;
    m_face.expressInterest(interest,
                           bind(&Producer::onData, this,  _1, _2),
                           bind(&Producer::onNack, this, _1, _2),
                           bind(&Producer::onTimeout, this, _1));

    // processEvents will block until the requested data is received or a timeout occurs
    m_face.processEvents();
  }

private:
	
	std::string
  fileLocation(const Interest& interest,const std::string dir,const std::string extension)
  {
    // cert file location
    std::string subName = interest.getName().getSubName(3).toUri();
    std::string keyPrefix = "/ndn/ca" + subName; 
    //std::cout << keyPrefix << std::endl; 
    std::string keyPrefix_hash = std::to_string(std::hash<std::string>{}(keyPrefix));      
    std::string location = dir + keyPrefix_hash + extension;
   
    return location;
  }
  
  void 
  writeFile(std::string loc,std::string content)
  {
    std::ofstream out(loc,std::ios::out | std::ios::trunc);
    out << content;
    out.close();
  }
  
  void
  onData(const Interest& interest, const Data& data) 
  {
    std::cout << "Received Data " << data << std::endl;
    /*
    char * content_c = new char[data.getContent().value_size() + 1];
    memcpy(content_c,data.getContent().value(),data.getContent().value_size());
    std::string content = content_c;
    //std::cout << content << std::endl;
    
    std::string loc = fileLocation(interest,"./certV2/",".cert");
    std::cout << loc << std::endl;
    writeFile(loc,content);
    */
    KeyIdType keyIdType = KeyIdType::SHA256;
    unique_ptr<KeyParams> params;
    params = make_unique<RsaKeyParams>(detail::RsaKeyParamsInfo::getDefaultSize(), keyIdType);
    
    security::v2::KeyChain keyChain;

    security::Identity identity;
    security::Key key;
    
    identity = keyChain.createIdentity(short_prefix, *params);
    key = identity.getDefaultKey();
    io::save(key.getDefaultCertificate(), "./signRequest/1.req");
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
  std::string short_prefix;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
	std::cout << "run" << std::endl;
  try {
    if (argc != 2 || argv[1][0] != '/') {
      std::cerr << "prefix must be specified,start with '/'" << std::endl;
      return 1;
    }
    ndn::examples::Producer producer(argv[1]);
    producer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
