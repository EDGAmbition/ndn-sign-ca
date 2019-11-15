/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include "ndn-cxx/security/v2/validation-error.hpp"
#include <iostream>
namespace ndn {
namespace examples {
class Consumer : noncopyable
{
public:

  Consumer(const std::string &fileName): m_validator(m_face)
  //Consumer(const std::string &fileName)
  {
      try {
        std::cout << "OPEN File= " << fileName << std::endl;
        m_validator.load(fileName);
      }
      catch (const std::exception &e ) {
        std::cout << "Can not load File= " << fileName << ". Error: " << e.what()
          <<  std::endl;
        exit(1);
      }

  }

  void
  run()
  { 
    // Request interest
    Interest interest(Name("/ghy/1/abp159.avi"));
    interest.setInterestLifetime(time::milliseconds(1000));
    interest.setMustBeFresh(true);

    m_face.expressInterest(interest,
                           bind(&Consumer::onData, this,  _1, _2),
                           bind(&Consumer::onNack, this, _1, _2),
                           bind(&Consumer::onTimeout, this, _1));

    std::cout << "I >> : " << interest << std::endl;
    //std::cout << "function getName():"<< interest.getName() << std::endl;
    // processEvents will block until the requested data received or timeout occurs
    m_face.processEvents();
  }

private:
  void
  onData(const Interest& interest, const Data& data)
  {
    std::cout << "D << : " << data << std::endl;

    // Validating data
    m_validator.validate(data,
                         bind(&Consumer::onValidated, this, _1),
                         bind(&Consumer::onValidationFailed, this, _1, _2));

  }
  
  void
  onNack(const Interest&, const lp::Nack& nack) const
  {
    std::cout << "Received Nack with reason " << nack.getReason() << std::endl;
  }

  void
  onTimeout(const Interest& interest)
  {
    std::cout << "Timeout " << interest << std::endl;
  }
  

  void 
  //onValidated(const shared_ptr<const Data>& data)
  onValidated(const Data& data)
  {
    std::cout << "Validated data: " << data.getName() << std::endl;
  }

  void 
  //onValidationFailed(const shared_ptr<const Data>& data, const std::string& failureInfo)
  onValidationFailed(const Data& data, const ndn::security::v2::ValidationError& error)
  {
    std::cerr << "Not validated data: " << data.getName() 
              << ". The failure info: " << error << std::endl;
  }

private:
  Face m_face;
  ValidatorConfig m_validator; 
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
 if (argc != 2) {
     std::cerr << "Validation file must be specified" << std::endl;
     std::cerr << "General use:" << std::endl;
     std::cerr << "  " << argv[0] << " validation_file" << std::endl;
     return 1;
  }

  ndn::examples::Consumer consumer(argv[1]);
  try {
    consumer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
