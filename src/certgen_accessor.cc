/*  =========================================================================
    certgen_accessor - accessor to interface with certgen library

    Copyright (C) 2014 - 2019 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#include "fty_certificate_generator_classes.h"

#include "certgen_accessor.h"

#include <fty_common_mlm.h>

//  Structure of our class
namespace certgen
{
    CertGenAccessor::CertGenAccessor(fty::SyncClient & reqClient)
        : m_requestClient (reqClient)
    {}

    void CertGenAccessor::generateSelfCertificateReq(
        const std::string & serviceName
    ) const
    {
        fty::Payload payload = sendCommand(GENERATE_SELFSIGNED_CERTIFICATE, {serviceName});
    }

    fty::CsrX509 CertGenAccessor::generateCsr(const std::string & serviceName) const
    {
        fty::Payload payload = sendCommand(GENERATE_CSR, {serviceName});

        return fty::CsrX509(payload[0]);
    }

    void CertGenAccessor::importCertificate(
        const std::string & serviceName,
        const std::string & cert
    ) const
    {
        fty::Payload payload = sendCommand(IMPORT_CERTIFICATE, {serviceName, cert});
    }

    // send helper function
    fty::Payload CertGenAccessor::sendCommand(
        const std::string & command,
        const fty::Payload & data
    ) const
    {
        fty::Payload payload = {command};

        std::copy(data.begin(), data.end(), back_inserter(payload));

        fty::Payload recMessage = m_requestClient.syncRequestWithReply(payload);

        if(recMessage[0] == "ERROR")
        {
            // error - throw exception
            if(recMessage.size() == 2)
            {
                throw std::runtime_error(recMessage.at(1));
            }
            else
            {
                throw std::runtime_error("Unknown error");
            }
        }

        return recMessage;    
    }
} // namescpace certgen

//  --------------------------------------------------------------------------
//  Test of this class => This is used by certgen_certificate_generator_server_test
//  --------------------------------------------------------------------------


std::vector<std::pair<std::string,bool>> certgen_accessor_test(mlm::MlmSyncClient & syncClient)
{
    std::vector<std::pair<std::string,bool>> testsResults;
  
    using namespace certgen;

    std::string testNumber, testName;

    //test 1.X
    {
        //test 1.1 => generate self signed certificate
        testNumber = "1.1";
        testName = "generateSelfCertificateReq => valid configuration file";
        printf("\n-----------------------------------------------------------------------\n");
        {
            printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
            try
            {
                CertGenAccessor accessor(syncClient);
                accessor.generateSelfCertificateReq("service-1");
                printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
                testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
            }
            catch(const std::exception& e)
            {
                printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                printf ("Error: %s\n", e.what ());
                testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
            }
        }
        
        //test 1.2 => generate self signed certificate (non existing config file)
        testNumber = "1.2";
        testName = "generateSelfCertificateReq => invalid configuration file";
        printf("\n-----------------------------------------------------------------------\n");
        {
            printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
            try
            {
                CertGenAccessor accessor(syncClient);
                accessor.generateSelfCertificateReq("fail");
                printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
                testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
            }
            catch(const std::runtime_error& e)
            {
                //expected error
                printf(" *<=  Test #%s > OK\n", testNumber.c_str());
                testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
            }
            catch(const std::exception& e)
            {
                printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
                printf("Error: %s\n",e.what());
                testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
            }
        }
    } // 1.X
    
    //test 2.X
    {
        //test 2.1 => generate self CSR
        testNumber = "2.1";
        testName = "generateCsr => success case";
        printf("\n-----------------------------------------------------------------------\n");
        {
            printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
            try
            {
                CertGenAccessor accessor(syncClient);
                fty::CsrX509 csr = accessor.generateCsr("service-1");
                printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
                testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
            }
            catch(const std::exception& e)
            {
                printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                printf ("Error: %s\n", e.what ());
                testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
            }
        }
        
        //test 2.2 => generate self signed certificate (non existing config file)
        testNumber = "2.2";
        testName = "generateCsr => create two requests for the same service";
        printf("\n-----------------------------------------------------------------------\n");
        {
            printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
            try
            {
                CertGenAccessor accessor(syncClient);
                fty::CsrX509 csr = accessor.generateCsr("service-1");

                fty::CsrX509 newCsr = accessor.generateCsr("service-1");
                if(newCsr.getPublicKey().getPem() == csr.getPublicKey().getPem())
                {
                    printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                    printf ("Error: %s\n", "Both requests have the same publicKey");
                    testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
                }
                else
                {
                    printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
                    testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
                }
            }
            catch(const std::exception& e)
            {
                printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                printf ("Error: %s\n", e.what ());
                testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
            }
        }
        
        //test 2.3 => generate self signed certificate (non existing config file)
        testNumber = "2.3";
        testName = "generateCsr => create two requests for two different services";
        printf("\n-----------------------------------------------------------------------\n");
        {
            printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
            try
            {
                CertGenAccessor accessor(syncClient);
                fty::CsrX509 csr1 = accessor.generateCsr("service-1");

                fty::CsrX509 csr2 = accessor.generateCsr("service-2");

                if(csr1.getPublicKey().getPem() == csr2.getPublicKey().getPem())
                {
                    printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                    printf ("Error: %s\n", "Both requests have the same publicKey");
                    testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
                }
                else
                {
                    printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
                    testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
                }
            }
            catch(const std::exception& e)
            {
                printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                printf ("Error: %s\n", e.what ());
                testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
            }
        }
    } // 2.X
    
    //test 3.X
    /*
    {
        //test 3.1 => import certificate
        testNumber = "3.1";
        testName = "importCertificate => valid configuration file";
        printf("\n-----------------------------------------------------------------------\n");
        {
            printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
            try
            {
                CertGenAccessor accessor(syncClient);
                fty::CsrX509 csr = accessor.generateCsr("service-1");

                fty::Keys keyPair(csr.getPublicKey().getPem());

                fty::CertificateConfig config;

                // TODO missing implementation in CertificateX509

                fty::CertificateX509 cert = fty::CertificateX509::signCsr(keyPair, config);

                accessor.importCertificate("service-1", cert.getPem());


                printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
                testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
            }
            catch(const std::exception& e)
            {
                printf (" *<=  Test #%s > Failed\n", testNumber.c_str ());
                printf ("Error: %s\n", e.what ());
                testsResults.emplace_back (" Test #" + testNumber + " " + testName, false);
            }
        }
    } // 3.X
    */
  

  return testsResults;
}