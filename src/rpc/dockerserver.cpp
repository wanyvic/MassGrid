// Copyright (c) 2014-2017 The MassGrid developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "base58.h"
#include "init.h"
#include "netbase.h"
#include "validation.h"
#include "coincontrol.h"
#include "masternode-payments.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "dockerpriceconfig.h"
#include "masternodeman.h"
#include "dockerman.h"
#include "dockerserverman.h"
#include "dockercluster.h"
#include "messagesigner.h"
#include "rpc/server.h"
#include "util.h"
#include "utilmoneystr.h"
#include "init.h"
#include "../wallet/wallet.h"
#include "dockercluster.h"
#include "dockeredge.h"
#include "timermodule.h"

#include <fstream>
#include <iomanip>
#include <univalue.h>


UniValue docker(const UniValue& params, bool fHelp)
{
    std::string strCommand;
    if (params.size() >= 1) {
        strCommand = params[0].get_str();
    }

#ifdef ENABLE_WALLET
    if (strCommand == "start-many")
        throw JSONRPCError(RPC_INVALID_PARAMETER, "DEPRECATED, please use start-all instead");
#endif // ENABLE_WALLET

    if (fHelp  ||
        (
#ifdef ENABLE_WALLET
            strCommand != "create" && strCommand != "delete"  && strCommand != "sendtomasternode" &&
#endif // ENABLE_WALLET
            strCommand != "connect" && strCommand != "disconnect" && strCommand != "getdndata"&& strCommand != "listprice" && strCommand != "listuntlementtx" && strCommand != "setprice"&& strCommand != "setdockerfee"))
            throw std::runtime_error(
                "docker \"command\"...\n"
                "Set of commands to execute docker related actions\n"
                "\nArguments:\n"
                "1. \"command\"        (string or set of strings, required) The command to execute\n"
                "\nAvailable commands:\n"
                "   getdndata      - \"dockernode (IP:Port)\" (string, required)  Print number of all of yours docker services\n"
                
                "   connect        - Connect to docker network\n"
                "   disconnect     - Disconnect to docker network\n"
#ifdef ENABLE_WALLET
                "   create         - create a docker service Arguments: \n"
                "                  \"dockernode (IP:Port)\" (string, required)\n"
                "                  \"service name\" (string, required)\n"
                "                  \"Image\"(string, required)\n"
                "                  \"CPU name\"(string, required)\n"
                "                  \"CPU thread count\"(int, required)\n"
                "                  \"Memory name\"(string, required)\n"
                "                  \"Memory GByte\"(int, required)\n"
                "                  \"GPU Kind\"(string, required)\n"
                "                  \"GPU count\"(int, required)\n"
                "                  \"NetWork Community\"(string, required)\n"
                "                  \"SSH_PUBKEY\"(string, required)\n"
                "   delete         - delete a docker service Arguments: \n"
                "                  \"dockernode (IP:Port)\" (string, required)\n"
                "                  \"txid (string, required)\n"
                "sendtomasternode  - send to masternode address: \n"
                "                  \"dockernode (IP:Port)\" (string, required)\n"
                "                  \"MassGrid address\"(string, required)\n"
                "                  \"amount\"(int, required)\n"
                "masternode command:  "
                "listuntlementtx   - list all untlement transactions\n"
                "listprice         - list all service items price\n"
                "setprice          - set item price\n"
                "                  \"type\" (string, required)\n"
                "                  \"name\"(string, required)\n"
                "                  \"price\"(double, required)\n"
                "setdockerfee      - set docker masternode fee (0.01):\n"
                "                  \"price\" (percent(double), required)\n"
                + HelpExampleCli("docker", "create \"119.3.66.159:19443\" \"1d1d4e24ed99057e84c3f80fd8fbec79ed9e1acee37da269356ecea000000000\" \"MassGrid\" \"massgrid/10.0-base-ubuntu16.04\" intel_i3 1 ddr 1 \"nvidia_p104_100_4g\" 1 \"massgridn2n\" \"ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDPEBGcs6VnDI89aVZHBCoDVq57qh7WamwXW4IbaIMWPeYIXQGAaYt83tCmJAcVggM176KELueh7+d1VraYDAJff9V5CxVoMhdJf1AmcIHGCyEjHRf12+Lme6zNVa95fI0h2tsryoYt1GAwshM6K1jUyBBWeVUdITAXGmtwco4k12QcDhqkfMlYD1afKjcivwaXVawaopdNqUVY7+0Do5ct4S4DDbx6Ka3ow71KyZMh2HpahdI9XgtzE3kTvIcena9GwtzjN+bf0+a8+88H6mtSyvKVDXghbGjunj55SaHZEwj+Cyv6Q/3EcZvW8q0jVuJu2AAQDm7zjgUfPF1Fwdv/ MassGrid\"")
                + HelpExampleCli("docker", "delete \"119.3.66.159:19443\" \"1d1d4e24ed99057e84c3f80fd8fbec79ed9e1acee37da269356ecea000000000\"")
                + HelpExampleCli("docker", "sendtomasternode \"119.3.66.159:19443\" \"mfb4XJGyaBwNK2Lf4a7r643U3JotRYNw2T\" 6.4")
                + HelpExampleCli("docker", "listuntlementtx")
                + HelpExampleCli("docker", "listprice")
                + HelpExampleCli("docker", "setprice \"cpu intel_i3 0.8\"")
                + HelpExampleCli("docker", "setdockerfee 0.01")
#endif // ENABLE_WALLET
                + HelpExampleCli("docker", "getdndata \"119.3.66.159:19443\"")
                + HelpExampleCli("docker", "connect \"massgridn2n\" \"10.1.1.4\" \"119.3.66.159\"")
                + HelpExampleCli("docker", "disconnect")
                );


    
    if (strCommand == "connect")
    {
        if (params.size() != 4)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invaid count parameters");

        std::string strCommunity = params[1].get_str();

        std::string strLocalAddr = params[2].get_str();

        std::string strSnAddr = params[3].get_str();

        if(ThreadEdgeStart(strCommunity,strLocalAddr,strSnAddr)){
            return "edge Start Successfully";
        }
        else
            return "edge Start Failed";
    }

    if (strCommand == "disconnect"){
        if (params.size() != 1)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invaid count parameters");
        ThreadEdgeStop();
        return "edge Stop Successfully";
    }

#ifdef ENABLE_WALLET
    if(strCommand == "create")
    {
        if (!masternodeSync.IsSynced())
            return "Need to Synced First";
        if (params.size() != 13)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid count parameter");
        std::string strAddr = params[1].get_str();
        if(!dockercluster.SetConnectDockerAddress(strAddr))
            throw JSONRPCError(RPC_CLIENT_INVALID_IP_OR_SUBNET, "Invalid IP");
        if(!dockercluster.ProcessDockernodeConnections())
            throw JSONRPCError(RPC_CLIENT_NODE_NOT_CONNECTED, "Connect to Masternode failed");

        DockerCreateService createService{};

        createService.pubKeyClusterAddress = dockercluster.DefaultPubkey;
        createService.txid = uint256S(params[2].get_str());
        
        std::string strServiceName = params[3].get_str();
        createService.serviceName = strServiceName;

        std::string strServiceImage = params[4].get_str();
        createService.image = strServiceImage;

        createService.item.cpu.Name = params[5].get_str();
        int64_t serviceCpu = params[6].get_int64();
        createService.item.cpu.Count = serviceCpu;

        createService.item.mem.Name = params[7].get_str();
        int64_t serviceMemoey_byte = params[8].get_int64();
        createService.item.mem.Count = serviceMemoey_byte;
        
        std::string strServiceGpuName = params[9].get_str();
        createService.item.gpu.Name = strServiceGpuName;

        int64_t serviceGpu = params[10].get_int64();
        createService.item.gpu.Count = serviceGpu;

        std::string strn2n_Community = params[11].get_str();
        createService.n2n_community = strn2n_Community;

        std::string strssh_pubkey = params[12].get_str();
        createService.ssh_pubkey = strssh_pubkey;
        EnsureWalletIsUnlocked();
        if(!dockercluster.CreateAndSendSeriveSpec(createService))
            return "CreateSpec Error Failed";

        for(int i=0;i<20;++i){
            MilliSleep(100);
            if(dockerServerman.getDNDataStatus() == CDockerServerman::DNDATASTATUS::Received){
                return strServiceCode[dockercluster.dndata.errCode];
            }
        }
        return NullUniValue;
    }
    if(strCommand == "delete"){
        if (!masternodeSync.IsSynced())
            return "Need to Synced First";
        if (params.size() != 3)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid count parameter");
        std::string strAddr = params[1].get_str();
        if(!dockercluster.SetConnectDockerAddress(strAddr))
            throw JSONRPCError(RPC_CLIENT_INVALID_IP_OR_SUBNET, "Invalid IP");
        if(!dockercluster.ProcessDockernodeConnections())
            throw JSONRPCError(RPC_CLIENT_NODE_NOT_CONNECTED, "Connect to Masternode failed");
        
        DockerDeleteService delService{};

        delService.pubKeyClusterAddress = dockercluster.DefaultPubkey;
        
        std::string strtxid = params[2].get_str();
        delService.txid = uint256S(strtxid);
        EnsureWalletIsUnlocked();
        CKey vchSecret;
        if (!pwalletMain->GetKey(delService.pubKeyClusterAddress.GetID(), vchSecret)){
            return "delService Error not found privkey";
        }
        if(!delService.Sign(vchSecret,delService.pubKeyClusterAddress)){
            return "delService Sign Error";
        }
        g_connman->PushMessage(dockercluster.connectNode, NetMsgType::DELETESERVICE, delService);

        for(int i=0;i<20;++i){
            MilliSleep(100);
            if(dockerServerman.getDNDataStatus() == CDockerServerman::DNDATASTATUS::Received){
                return strServiceCode[dockercluster.dndata.errCode];
            }
        }
        return NullUniValue;
    }
    if(strCommand == "listuntlementtx"){
        if (!fDockerNode)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "This is not a masternode");
        UniValue varr(UniValue::VARR);
        std::set<CWalletTx*> setWallet = timerModule.GetWalletTxSet();
        for(const auto& tx :setWallet)
            varr.push_back(tx->GetHash().ToString());
        return varr;
    }
    if(strCommand == "listprice"){
        if (!fDockerNode)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "This is not a masternode");
        UniValue varr(UniValue::VARR);
        auto entries = dockerPriceConfig.getEntries();
        int entriesNum=1;
        for(auto it = entries.begin();it!= entries.end();++it){
        UniValue obj(UniValue::VOBJ);
        std::ostringstream streamFull;
        streamFull << it->getType() << " " <<
                        it->getName() << " " <<
                        (double)it->getPrice()/COIN;
        std::string strFull = streamFull.str();
        obj.push_back(Pair(std::to_string(entriesNum++),strFull));
        varr.push_back(obj);
        }
        return varr;
    }
    if(strCommand == "setprice"){
        if (!fDockerNode)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "This is not a masternode");
        if (params.size() != 4)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid count parameter");
        std::string type = params[1].get_str();
        std::string name = params[2].get_str();
        std::string strPrice = params[3].get_str();
        auto entries = dockerPriceConfig.getEntries();
        for(auto it = entries.begin();it!= entries.end();++it){
            if(type == it->getType() && name == it->getName()){
                it->setPrice(strPrice);    
                return "modified " + it->getType() + " " +it->getName() + " " +std::to_string(it->getPrice());
            }
        }
        dockerPriceConfig.add(type,name,strPrice);
        return "add new "+type + " " +name + " " +strPrice;
    }    
    if (strCommand == "setdockerfee"){
        if (!fDockerNode)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "This is not a masternode");
        if (params.size() < 2)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Please specify an fee rate");
        std::string strfeeRate = params[1].get_str();
        double fee;
        try
        {
            fee = boost::lexical_cast<double>(strfeeRate);
        }
        catch(const boost::bad_lexical_cast &)
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "bad_lexical_cast");
        }
        if(fee >=1 || fee <0)
            return "docker feerate set failed feeRate is :"+std::to_string(fee); 
        dockerServerman.feeRate = fee;
        return "docker feerate is" + std::to_string(dockerServerman.feeRate);
    }
    if(strCommand == "sendtomasternode"){
        LOCK2(cs_main, pwalletMain->cs_wallet);
        std::string strAddr = params[1].get_str();
        std::string mnoupouint;
        std::map<COutPoint, CMasternode> mapMasternodes = mnodeman.GetFullMasternodeMap();
        for(auto it = mapMasternodes.begin();it!=mapMasternodes.end();++it){
            if(it->second.addr.ToString() == strAddr){
                mnoupouint = it->first.ToStringShort();
                break;
            }
        }
        CMassGridAddress address(params[2].get_str());
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MassGrid address");

        // Amount
        CAmount nAmount = AmountFromValue(params[3]);
        if (nAmount <= 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

        // Wallet comments
        CWalletTx wtxNew;
        if(mnoupouint.empty()){
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "MassGrid masternode outpoint not found");
        }
        wtxNew.Setmasternodeoutpoint(mnoupouint);
        wtxNew.Setmasternodeip(strAddr);
        wtxNew.Setmasternodeaddress(params[2].get_str());

        EnsureWalletIsUnlocked();

        CAmount curBalance = pwalletMain->GetBalance();

        // Check amount

        if (nAmount > curBalance)
            throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

        if (pwalletMain->GetBroadcastTransactions() && !g_connman)
            throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

        // Parse MassGrid address
        CScript scriptPubKey = GetScriptForDestination(address.Get());

        // Create and send the transaction
        CReserveKey reservekey(pwalletMain);
        CAmount nFeeRequired;
        std::string strError;
        vector<CRecipient> vecSend;
        int nChangePosRet = -1;
        CRecipient recipient = {scriptPubKey, nAmount, false};
        vecSend.push_back(recipient);

        CCoinControl coinControl;
        coinControl.fUseInstantSend = true;
        coinControl.destChange = CMassGridAddress(pwalletMain->vchDefaultKey.GetID()).Get();

        if (!pwalletMain->CreateTransaction(vecSend, wtxNew, reservekey, nFeeRequired, nChangePosRet,
                                            strError, &coinControl, true,ALL_COINS, true)) {
            if (nAmount + nFeeRequired > pwalletMain->GetBalance())
                strError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        if (!pwalletMain->CommitTransaction(wtxNew, reservekey, g_connman.get(), NetMsgType::TXLOCKREQUEST))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");


        return wtxNew.GetHash().GetHex();
    }
#endif // ENABLE_WALLET
    
    if (strCommand == "getdndata"){
        if (!masternodeSync.IsSynced())
            return "Need to Synced First";
        
        if (params.size() != 2)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invaid count parameters");
        std::string strIPPort;
        strIPPort = params[1].get_str();
        if(!dockercluster.SetConnectDockerAddress(strIPPort)){
            throw JSONRPCError(RPC_INVALID_PARAMETER, "SetConnectDockerAddress error");
        }
        if(!dockercluster.ProcessDockernodeConnections()){
            throw JSONRPCError(RPC_INVALID_PARAMETER, "ProcessDockernodeConnections error");
        }
        dockercluster.AskForDNData();

        UniValue varr(UniValue::VARR);
        for(int i=0;i<20;++i){
            MilliSleep(100);
            if(dockerServerman.getDNDataStatus() == CDockerServerman::DNDATASTATUS::Received){
                for(auto it = dockercluster.dndata.mapDockerServiceLists.begin();it != dockercluster.dndata.mapDockerServiceLists.end();++it){
                    UniValue obj(UniValue::VOBJ);
                    obj.push_back(Pair(it->first,Service::DockerServToJson(it->second)));
                    varr.push_back(obj);
                }
                UniValue varr2(UniValue::VARR);
                for(auto it = dockercluster.dndata.items.begin();it!= dockercluster.dndata.items.end();++it){
                    UniValue obj(UniValue::VOBJ);
                    obj.push_back(Pair("CpuType",it->first.cpu.Name));
                    obj.push_back(Pair("CpuCount",it->first.cpu.Count));
                    obj.push_back(Pair("MemSize",it->first.mem.Count));
                    obj.push_back(Pair("GpuType",it->first.gpu.Name));
                    obj.push_back(Pair("GpuCount",it->first.gpu.Count));
                    obj.push_back(Pair("Price",(double)it->second.price/COIN));
                    obj.push_back(Pair("UsageCount",it->second.count));
                    varr2.push_back(obj);
                }
                varr.push_back(varr2);
                UniValue obj(UniValue::VOBJ);
                obj.push_back(Pair("masternodeaddress",dockercluster.dndata.masternodeAddress));
                varr.push_back(obj);
                return varr;
            }
        }
    }
    return NullUniValue;
}
