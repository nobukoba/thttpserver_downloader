#include "TXMLEngine.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TBufferJSON.h"
#include "TSystem.h"
#include "TString.h"
#include <stdio.h>
#include <iostream>

void GotoNode(TXMLEngine &xml, XMLNodePointer_t node, Int_t level, TString baseurl, TString fulldir) {
   // this function display all accessible information about xml node and its children
   //printf("%*c node: %s\n", level, ' ', xml.GetNodeName(node));
   //
   // display namespace
   //XMLNsPointer_t ns = xml.GetNS(node);
   //if (ns != 0)
   //   printf("%*c namespace: %s refer: %s\n", level + 2, ' ', xml.GetNSName(ns), xml.GetNSReference(ns));
   //
   //// display attributes
   //XMLAttrPointer_t attr = xml.GetFirstAttr(node);
   //while (attr != 0) {
   //   printf("%*c attr: %s value: %s\n", level + 2, ' ', xml.GetAttrName(attr), xml.GetAttrValue(attr));
   //   attr = xml.GetNextAttr(attr);
   //}
   //
   //// display content (if exists)
   //const char *content = xml.GetNodeContent(node);
   //if (content != 0)
   //   printf("%*c cont: %s\n", level + 2, ' ', content);

   TDirectory *save = gDirectory;
   TString attr_kind = xml.GetAttr(node,"_kind");
   TString attr_name = xml.GetAttr(node,"_name");
   if (attr_kind.BeginsWith("ROOT.TH")) {
     TString cmd = "curl -s " + baseurl + "/"+ fulldir + attr_name + "/root.json.gz | gunzip -c";
     cmd.ReplaceAll(";","\\;");
     cmd.ReplaceAll("(","\\(");
     cmd.ReplaceAll(")","\\)");
     std::cout << cmd << std::endl;
     TString result = gSystem->GetFromPipe(cmd.Data());
     TObject *obj = 0;
     TBufferJSON::FromJSON(obj, result);
     if (obj) obj->Write();
   }else{
     TDirectoryFile * dir = new TDirectoryFile(attr_name.Data(),attr_name.Data(),"",save);
     dir->cd();
     fulldir += attr_name + "/";
   }

   // display all child nodes
   XMLNodePointer_t child = xml.GetChild(node);
   while (child != 0) {
     GotoNode(xml, child, level + 2, baseurl, fulldir);
      child = xml.GetNext(child);
   }
   save->cd();
   return;
}

int main(int argc, char **argv) {
  /* rpath is used for this program. If LD_LIBRARY_PATH is set,
     the other version of root lib will be loaded. Therefore
     the LD_LIBRARY_PATH should be unset. */
  gSystem->Setenv("LD_LIBRARY_PATH",""); 

  TString baseurl = "http://oxygen.rcnp.osaka-u.ac.jp:8083/";
  if (argc > 1) {
    baseurl = argv[1];
  }
    
  // First create engine
  TXMLEngine xml;
  
  // Now try to parse xml file
  // Only file with restricted xml syntax are supported
  TString cmd = "curl -s " + baseurl + "/h.xml";
  TString result = gSystem->GetFromPipe(cmd.Data());
  //   XMLDocPointer_t xmldoc = xml.ParseFile(filename);
  XMLDocPointer_t xmldoc = xml.ParseString(result);
  if (!xmldoc) return 0;
  
  // take access to main node
  XMLNodePointer_t mainnode = xml.DocGetRootElement(xmldoc);
  XMLNodePointer_t child = xml.GetChild(mainnode);
  TString filetitle = xml.GetAttr(child,"_name");
  TString filename = filetitle + ".root";
  TFile* f = new TFile(filename.Data(),"recreate");
  std::cout << "New ROOT file: " << filename.Data() << " was created." << std::endl;
  
  child = xml.GetChild(child);
  // display recursively all nodes and subnodes
  TString fulldir = "";
  while (child != 0) {
    GotoNode(xml, child, 1, baseurl, fulldir);
    child = xml.GetNext(child);
  }
  
  // Release memory before exit
  xml.FreeDoc(xmldoc);
  
  f->Write();
  f->Close();
  return 0;
}
