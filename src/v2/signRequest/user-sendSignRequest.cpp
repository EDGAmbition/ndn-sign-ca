/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2019 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespaces should be used to prevent/limit name conflicts
namespace examples {

class Producer
{
public:
  void
  run()
  {
    m_face.setInterestFilter("/ghy/1/getSignRequest",
                             bind(&Producer::onInterest, this, _1, _2),
                             nullptr, // RegisterPrefixSuccessCallback is optional
                             bind(&Producer::onRegisterFailed, this, _1, _2));
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
  
  std::string 
  readFile(const char *pFileName)
  {
	  std::ifstream inFile(pFileName);
	  std::string contents("");
	  if (inFile.is_open())
	  {
		  std::stringstream buffer;
		  buffer << inFile.rdbuf();
		  contents.append(buffer.str());	
	  } 
	    
	  inFile.close();   
	  return contents;
  }
  
  void
  onInterest(const InterestFilter&, const Interest& interest)
  {
    std::cout << ">> I: " << interest << std::endl;

    std::string content = readFile("./1.req");
    // Create Data packet
    auto data = make_shared<Data>(interest.getName());
    data->setFreshnessPeriod(10_s);
    data->setContent(reinterpret_cast<const uint8_t*>(content.data()), content.size());
    
    m_keyChain.sign(*data, signingWithSha256());

    // Return Data packet to the requester
    std::cout << "<< D: " << *data << std::endl;
    m_face.put(*data);
  }

  void
  onRegisterFailed(const Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix '" << prefix
              << "' with the local forwarder (" << reason << ")" << std::endl;
    m_face.shutdown();
  }

private:
  Face m_face;
  KeyChain m_keyChain;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  try {
    ndn::examples::Producer producer;
    producer.run();
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
