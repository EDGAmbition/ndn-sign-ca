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
#include <ndn-cxx/util/time.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <functional>
#include <string>
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
    m_face.setInterestFilter("/ndn/ca",
                             bind(&Producer::onInterest, this, _1, _2),
                             nullptr, // RegisterPrefixSuccessCallback is optional
                             bind(&Producer::onRegisterFailed, this, _1, _2));
    m_face.processEvents();
  }

private:
  
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
  
  char *
  fileLocation(const Interest& interest)
  {
    // cert file location
    std::string subName = interest.getName().getSubName(3).toUri();
    std::string keyPrefix = "/ndn/ca" + subName; 
    //std::cout << keyPrefix << std::endl; 
    std::string keyPrefix_hash = std::to_string(std::hash<std::string>{}(keyPrefix));      
    std::string location = "./key/" + keyPrefix_hash + ".cert";
    
    char * location_c = new char [location.length() + 1];
    strcpy(location_c,location.c_str());
    
    return location_c;
  }
  
  std::string 
  timeFormat(ndn::time::system_clock::TimePoint time){
	  std::string time_str = toIsoString(time);
	  std::string time_str1 = time_str.substr(0,8);
    std::string time_str2 = time_str.substr(9,6);
    time_str = time_str1 + time_str2;
    return time_str;
  }
  
  void 
  runCmd(std::string cmd){
    char *cmd_c = new char[cmd.length() + 1];
    strcpy(cmd_c,cmd.c_str());
    system(cmd_c);
    delete [] cmd_c;
  }
  void 
  genKeyAndCert(const Interest& interest)
  {
    // generate a key for this prefix
    std::string subName = interest.getName().getSubName(3).toUri();
    std::string keyPrefix = "/ndn/ca" + subName;  
    //std::cout << "KEY prefix :"<< keyPrefix << std::endl;
    
    // get hash of keyPrefix
    std::string keyPrefix_hash = std::to_string(std::hash<std::string>{}(keyPrefix)); 
      
    std::string cmdKeyGen = "ndnsec-key-gen -n " + keyPrefix + 
                            " >./key/" + keyPrefix_hash + ".key";
    runCmd(cmdKeyGen);
    /*
     *  generate a certificate 
     */
    // timestamp
    time::system_clock::TimePoint startTime = time::system_clock::now();
    std::string startTimeFormat = timeFormat(startTime);
    
    uint64_t timeUint64 = toUnixTimestamp(startTime).count();
    timeUint64 += 604800000; // magic number = one week 
    time::system_clock::TimePoint endTime = 
  	  time::fromUnixTimestamp(boost::chrono::milliseconds(timeUint64));
  	std::string endTimeFormat = timeFormat(endTime);
  	
  	std::string cmdCertGen = "ndnsec-cert-gen -S " + startTimeFormat + 
  	                         " -E " + endTimeFormat +
  	                         " -I \"Certificate issued by the BCCA\"" +
  	                         " -s " + keyPrefix + 
  	                         " -i \"BCCA\"" +
  	                         " ./key/" + keyPrefix_hash + ".key" // which file
  	                         " > ./key/" + keyPrefix_hash + ".cert"
  	                         ;
    //std::cout << cmdCertGen << std::endl;
    runCmd(cmdCertGen);
    std::cout << "ok!!!" <<std::endl;
  }
  void
  onInterest(const InterestFilter&, const Interest& interest)
  {
    std::string whichRequest = interest.getName().getSubName(2,1).toUri();
    std::cout << whichRequest << std::endl;
    std::string content;
    if (whichRequest == "/getPrefix"){
      //std::cout << "1" << std::endl;
      content = "ojbk!";
      genKeyAndCert(interest);
         
    }
    else if(whichRequest == "/getCert"){
      //std::cout << "2" << std::endl;
      char * certLocation = fileLocation(interest);
      
      content = readFile(certLocation);
      //std::cout << content << std::endl;     
    }    
    // Create Data packet
    auto data = make_shared<Data>(interest.getName());
    data->setFreshnessPeriod(10_s);
    data->setContent(reinterpret_cast<const uint8_t*>(content.data()), content.size());
    // sign
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
