#include "TXMLEngine.h"
#include "TSystem.h"
#include "TString.h"
#include <stdio.h>
#include <iostream>

void DisplayNode(TXMLEngine &xml, XMLNodePointer_t node, Int_t level)
{
   // this function display all accessible information about xml node and its children

   printf("%*c node: %s\n", level, ' ', xml.GetNodeName(node));

   // display namespace
   XMLNsPointer_t ns = xml.GetNS(node);
   if (ns != 0)
      printf("%*c namespace: %s refer: %s\n", level + 2, ' ', xml.GetNSName(ns), xml.GetNSReference(ns));

   // display attributes
   XMLAttrPointer_t attr = xml.GetFirstAttr(node);
   while (attr != 0) {
      printf("%*c attr: %s value: %s\n", level + 2, ' ', xml.GetAttrName(attr), xml.GetAttrValue(attr));
      attr = xml.GetNextAttr(attr);
   }

   // display content (if exists)
   const char *content = xml.GetNodeContent(node);
   if (content != 0)
      printf("%*c cont: %s\n", level + 2, ' ', content);

   // display all child nodes
   XMLNodePointer_t child = xml.GetChild(node);
   while (child != 0) {
      DisplayNode(xml, child, level + 2);
      child = xml.GetNext(child);
   }
   return;
}

int main(int argc, char **argv) {
  std::cout << "aa" << std::endl;
   // First create engine
   TXMLEngine xml;

   // Now try to parse xml file
   // Only file with restricted xml syntax are supported
  TString cmd = "curl -s http://oxygen.rcnp.osaka-u.ac.jp:8083/h.xml";
  TString result = gSystem->GetFromPipe(cmd.Data());
  //   XMLDocPointer_t xmldoc = xml.ParseFile(filename);
   XMLDocPointer_t xmldoc = xml.ParseString(result);
   if (!xmldoc) return 0;

   // take access to main node
   XMLNodePointer_t mainnode = xml.DocGetRootElement(xmldoc);

   // display recursively all nodes and subnodes
   DisplayNode(xml, mainnode, 1);

   // Release memory before exit
   xml.FreeDoc(xmldoc);
  
  return 0;
}
