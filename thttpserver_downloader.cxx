#include "TXMLEngine.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TBufferJSON.h"
#include "TBufferXML.h"
#include "TSystem.h"
#include "TString.h"
#include <stdio.h>
#include <iostream>

void GotoNode(TXMLEngine &xml, XMLNodePointer_t node, Int_t level,
	      TString baseurl, TString fulldir, TDirectoryFile* curdir) {
  TString attr_kind = xml.GetAttr(node,"_kind");
  TString attr_name = xml.GetAttr(node,"_name");
  TString attr_typename = xml.GetAttr(node,"_typename");
  TDirectoryFile* newdir = 0;
  if (attr_kind.BeginsWith("ROOT.TH")) {
    TString cmd = "";
    if (level == 1) {
      cmd = "curl -s " + baseurl + "/root.json";
    }else{
      cmd = "curl -s " + baseurl + "/" + fulldir + "/" + attr_name + "/root.json";
    }
    cmd.ReplaceAll(";","\\;");
    cmd.ReplaceAll("(","\\(");
    cmd.ReplaceAll(")","\\)");
    std::cout << cmd << std::endl;
    TString result = gSystem->GetFromPipe(cmd.Data());
    TObject *obj = 0;
    curdir->cd();
    TBufferJSON::FromJSON(obj, result);
    if (obj) {
      obj->Write();
    }
  }else if(attr_typename.EqualTo("")){
    /*}else{*/
    if ((level == 1) && baseurl.EndsWith("/")) {
      newdir = curdir;
    }else{
      curdir->cd();
      newdir = new TDirectoryFile(attr_name.Data(),attr_name.Data(),"",curdir);
    }
    if (level > 1){
      fulldir += attr_name + "/";
    }
    XMLNodePointer_t child = xml.GetChild(node);
    while (child != 0) {
      GotoNode(xml, child, level + 1, baseurl, fulldir, newdir);
      child = xml.GetNext(child);
    }
  }
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
    
  /* First create engine */
  TXMLEngine xml;
  
  /* Now try to parse xml file
     Only file with restricted xml syntax are supported */
  TString cmd = "curl -s " + baseurl + "/h.xml";
  cmd.ReplaceAll(";","\\;");
  cmd.ReplaceAll("(","\\(");
  cmd.ReplaceAll(")","\\)");
  TString result = gSystem->GetFromPipe(cmd.Data());
  XMLDocPointer_t xmldoc = xml.ParseString(result);
  if (!xmldoc) return 0;
  
  /* take access to main node */
  XMLNodePointer_t mainnode = xml.DocGetRootElement(xmldoc);
  XMLNodePointer_t child = xml.GetChild(mainnode);

  TString filetitle = xml.GetAttr(child,"_name");
  TString filename = filetitle + ".root";
  TFile* f = new TFile(filename.Data(),"recreate");
  std::cout << "New ROOT file: " << filename.Data() << " was created." << std::endl;

  /* Go to recursively all nodes and subnodes */
  TDirectoryFile* curdir = (TDirectoryFile*)f;
  TString fulldir = "";
  GotoNode(xml, child, 1, baseurl, fulldir, curdir);
  
  /* Release memory before exit */
  xml.FreeDoc(xmldoc);
  
  f->Write();
  f->Close();
  return 0;
}
