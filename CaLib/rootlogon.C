{
  gSystem->Load("libGui.so");

  // The CaLib dictionary contains inline constructors which use TCConfig and
  // TCReadConfig.  Make these declarations known to Cling before loading the
  // PCM; otherwise ROOT 6.28 may replay only their forward declarations.
  gInterpreter->AddIncludePath("include");
  gInterpreter->Declare("#include \"TCConfig.h\"\n"
                        "#include \"TCReadConfig.h\"");

  if ( !gSystem->Load("../build/lib/libCaLib.so") )
    cout << "libCaLib.so is loaded" << endl;

}
