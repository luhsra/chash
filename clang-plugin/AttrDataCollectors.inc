DEF_ADD_DATA(Attr, {

    std::string AttrString;      
    llvm::raw_string_ostream OS(AttrString);
    S->printPretty(OS, Context.getLangOpts());
    OS.flush();
    addData(AttrString);
  }
)
#undef DEF_ADD_DATA
