/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

#include <iostream>

namespace ndn {
namespace examples {

class Producer : noncopyable
{
public:
  Producer(const std::string &identity): m_identity(identity)
  {
  }

  void
  run()
  {
    std::cout << "Data will be signed by: " << m_identity << std::endl;
    std::cout << "Interest Filter:  /example/test" << std::endl;

    m_face.setInterestFilter("/example/test",
                             bind(&Producer::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&Producer::onRegisterFailed, this, _1, _2));
    m_face.processEvents();
  }

private:


  void
  onInterest(const InterestFilter& filter, const Interest& interest)
  {
    std::cout << "<< I: " << interest << std::endl;

    // Create new name, based on Interest's name
    Name dataName(interest.getName());
    dataName
      .append("testApp") // add "testApp" component to Interest name
      .appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)

    static const std::string content = "DATA SIGNED with /ndn/keys/alice identity";

    // Create Data packet
    shared_ptr<Data> data = make_shared<Data>();
    data->setName(dataName);
    data->setFreshnessPeriod(time::seconds(10));
    data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
    
    
    m_keyChain.sign(*data, signingByIdentity(m_identity));

    // Return Data packet to the requester
    std::cout << ">> D: " << *data << std::endl;
    std::cout << "Signature. KeyLocator: " <<
                data->getSignature().getKeyLocator().getName() << std::endl;


    m_face.put(*data);
  }


  void
  onRegisterFailed(const Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
    m_face.shutdown();
  }

private:
  Face m_face;
  KeyChain m_keyChain;
  Name m_identity;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  if (argc != 2) {
     std::cerr << "Identity name must be specified" << std::endl;
     std::cerr << "General use:" << std::endl;
     std::cerr << "  " << argv[0] << " identity_to_sign_data" << std::endl;
     return 1;
  }

  std::string identity(argv[1]);

  ndn::examples::Producer producer(identity);
  try {
    producer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
