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

#include <iostream>
#include <string>
#include <fstream>
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespaces should be used to prevent/limit name conflicts
namespace examples {

class Consumer
{
public:
  Consumer(const std::string &prefix)
  {
    const std::string caPrefix_getCert = "ndn/ca/getCert"; 
    m_prefix = caPrefix_getCert + prefix;
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
    //std::cout << "content : " << data.getContent() <<std::endl;
    char * content_c = new char[data.getContent().value_size() + 1];
    memcpy(content_c,data.getContent().value(),data.getContent().value_size());
    std::string content = content_c;
    std::cout << content << std::endl;
    
    std::string loc = fileLocation(interest,"./cert/",".cert");
    writeFile(loc,content);
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

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{    
  try {
    if (argc != 2 || argv[1][0] != '/') {
      std::cerr << "prefix must be specified,start with '/'" << std::endl;
      return 1;
    }
    ndn::examples::Consumer consumer(argv[1]);
    consumer.run();
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
