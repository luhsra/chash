//--- Attributes ---------------------------------------------------------//
class Attr {
  code Code = [{
    std::string AttrString;      
    llvm::raw_string_ostream OS(AttrString);
    S->printPretty(OS, Context.getLangOpts());
    OS.flush();
    addData(AttrString);
  }];
}
