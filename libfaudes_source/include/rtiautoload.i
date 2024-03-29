/* rti2code: autogenerated libFAUDES luabindings declarations: libFAUDES 2.29d timed-iodevice-simulator */

/* faudes-function "Accessible" */
#if SwigModule == "SwigCoreFaudes"
void Accessible(Generator&);
SwigHelpEntry("Functions", "Reachability", " Accessible(+InOut+ Generator G)");
void Accessible(const Generator&, Generator&);
SwigHelpEntry("Functions", "Reachability", " Accessible(+In+ Generator GArg, +Out+ Generator GRes)");
#endif 

/* faudes-function "AlphabetDifference" */
#if SwigModule == "SwigCoreFaudes"
%rename(AlphabetDifference) SetDifference;
void SetDifference(const EventSet&, const EventSet&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetDifference(+In+ EventSet Sigma1, +In+ EventSet Sigma2, +Out+ EventSet Sigma)");
#endif 

/* faudes-function "AlphabetEquality" */
#if SwigModule == "SwigCoreFaudes"
%rename(AlphabetEquality) SetEquality;
bool SetEquality(const EventSet&, const EventSet&);
SwigHelpEntry("EventSet", "Functions", "Boolean AlphabetEquality(+In+ EventSet Sigma1, +In+ EventSet Sigma2)");
#endif 

/* faudes-function "AlphabetExtract" */
#if SwigModule == "SwigCoreFaudes"
void AlphabetExtract(const Generator&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetExtract(+In+ Generator Gen, +Out+ EventSet Sigma)");
#endif 

/* faudes-function "AlphabetInclusion" */
#if SwigModule == "SwigCoreFaudes"
%rename(AlphabetInclusion) SetInclusion;
bool SetInclusion(const EventSet&, const EventSet&);
SwigHelpEntry("EventSet", "Functions", "Boolean AlphabetInclusion(+In+ EventSet Sigma1, +In+ EventSet Sigma2)");
#endif 

/* faudes-function "AlphabetIntersection" */
#if SwigModule == "SwigCoreFaudes"
%rename(AlphabetIntersection) SetIntersection;
void SetIntersection(const EventSet&, const EventSet&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetIntersection(+In+ EventSet Sigma1, +In+ EventSet Sigma2, +Out+ EventSet Sigma)");
void SetIntersection(const EventSetVector&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetIntersection(+In+ EventSetVector AVec, +Out+ EventSet ARes)");
void SetIntersection(const GeneratorVector&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetIntersection(+In+ GeneratorVector GVec, +Out+ EventSet ARes)");
#endif 

/* faudes-function "AlphabetLanguage" */
#if SwigModule == "SwigCoreFaudes"
void AlphabetLanguage(const EventSet&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " AlphabetLanguage(+In+ EventSet Sigma, +Out+ Generator GRes)");
#endif 

/* faudes-function "AlphabetUnion" */
#if SwigModule == "SwigCoreFaudes"
%rename(AlphabetUnion) SetUnion;
void SetUnion(const EventSet&, const EventSet&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetUnion(+In+ EventSet Sigma1, +In+ EventSet Sigma2, +Out+ EventSet Sigma)");
void SetUnion(const EventSetVector&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetUnion(+In+ EventSetVector AVec, +Out+ EventSet ARes)");
void SetUnion(const GeneratorVector&, EventSet&);
SwigHelpEntry("EventSet", "Functions", " AlphabetUnion(+In+ GeneratorVector GVec, +Out+ EventSet ARes)");
#endif 

/* faudes-function "Automaton" */
#if SwigModule == "SwigCoreFaudes"
void Automaton(Generator&);
SwigHelpEntry("Functions", "Generator misc", " Automaton(+InOut+ Generator GArg)");
void Automaton(Generator&, const EventSet&);
SwigHelpEntry("Functions", "Generator misc", " Automaton(+InOut+ Generator GArg, +In+ EventSet Sigma)");
#endif 

/* faudes-function "Coaccessible" */
#if SwigModule == "SwigCoreFaudes"
void Coaccessible(Generator&);
SwigHelpEntry("Functions", "Reachability", " Coaccessible(+InOut+ Generator G)");
void Coaccessible(const Generator&, Generator&);
SwigHelpEntry("Functions", "Reachability", " Coaccessible(+In+ Generator GArg, +Out+ Generator GRes)");
#endif 

/* faudes-function "Complete" */
#if SwigModule == "SwigCoreFaudes"
void Complete(Generator&);
SwigHelpEntry("Functions", "Reachability", " Complete(+InOut+ Generator G)");
void Complete(const Generator&, Generator&);
SwigHelpEntry("Functions", "Reachability", " Complete(+In+ Generator G, +Out+ Generator GRes)");
void Complete(Generator&, const EventSet&);
SwigHelpEntry("Functions", "Reachability", " Complete(+InOut+ Generator G, +In+ EventSet SigmaO)");
void Complete(const Generator&, const EventSet&, Generator&);
SwigHelpEntry("Functions", "Reachability", " Complete(+In+ Generator G, +In+ EventSet SigmaO, +Out+ Generator GRes)");
#endif 

/* faudes-function "Deterministic" */
#if SwigModule == "SwigCoreFaudes"
%rename(Deterministic) aDeterministic;
void aDeterministic(const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " Deterministic(+In+ Generator G, +Out+ Generator GRes)");
#endif 

/* faudes-function "EmptyLanguage" */
#if SwigModule == "SwigCoreFaudes"
void EmptyLanguage(const EventSet&, Generator&);
SwigHelpEntry("Functions", "Language misc", " EmptyLanguage(+In+ EventSet Sigma, +Out+ Generator GRes)");
#endif 

/* faudes-function "EmptyStringLanguage" */
#if SwigModule == "SwigCoreFaudes"
void EmptyStringLanguage(const EventSet&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " EmptyStringLanguage(+In+ EventSet Sigma, +Out+ Generator GRes)");
#endif 

/* faudes-function "FullLanguage" */
#if SwigModule == "SwigCoreFaudes"
void FullLanguage(const EventSet&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " FullLanguage(+In+ EventSet Sigma, +Out+ Generator GRes)");
#endif 

/* faudes-function "IntegerSum" */
#if SwigModule == "SwigCoreFaudes"
long int IntegerSum(const long int&, const long int&);
SwigHelpEntry("Integer", "Elemetary types", "Integer IntegerSum(+In+ Integer I_1, +In+ Integer I_2)");
long int IntegerSum(const IntegerVector&);
SwigHelpEntry("Integer", "Elemetary types", "Integer IntegerSum(+In+ IntegerVector IVec)");
#endif 

/* faudes-function "InvProject" */
#if SwigModule == "SwigCoreFaudes"
%rename(InvProject) aInvProject;
void aInvProject(Generator&, const EventSet&);
SwigHelpEntry("Functions", "Language misc", " InvProject(+InOut+ Generator G0, +In+ EventSet Sigma)");
void aInvProject(const Generator&, const EventSet&, Generator&);
SwigHelpEntry("Functions", "Language misc", " InvProject(+In+ Generator G0, +In+ EventSet Sigma, +Out+ Generator GRes)");
#endif 

/* faudes-function "IsAccessible" */
#if SwigModule == "SwigCoreFaudes"
bool IsAccessible(const Generator&);
SwigHelpEntry("Functions", "Reachability", "Boolean IsAccessible(+In+ Generator G)");
#endif 

/* faudes-function "IsCoaccessible" */
#if SwigModule == "SwigCoreFaudes"
bool IsCoaccessible(const Generator&);
SwigHelpEntry("Functions", "Reachability", "Boolean IsCoaccessible(+In+ Generator G)");
#endif 

/* faudes-function "IsComplete" */
#if SwigModule == "SwigCoreFaudes"
bool IsComplete(const Generator&);
SwigHelpEntry("Functions", "Reachability", "Boolean IsComplete(+In+ Generator G)");
bool IsComplete(const Generator&, const EventSet&);
SwigHelpEntry("Functions", "Reachability", "Boolean IsComplete(+In+ Generator G, +In+ EventSet SigmaO)");
#endif 

/* faudes-function "IsDeterministic" */
#if SwigModule == "SwigCoreFaudes"
bool IsDeterministic(const Generator&);
SwigHelpEntry("Functions", "Generator misc", "Boolean IsDeterministic(+In+ Generator G)");
#endif 

/* faudes-function "IsEmptyLanguage" */
#if SwigModule == "SwigCoreFaudes"
bool IsEmptyLanguage(const Generator&);
SwigHelpEntry("Functions", "Language misc", "Boolean IsEmptyLanguage(+In+ Generator GArg)");
#endif 

/* faudes-function "IsNonblocking" */
#if SwigModule == "SwigCoreFaudes"
bool IsNonblocking(const Generator&);
SwigHelpEntry("Functions", "Composition", "Boolean IsNonblocking(+In+ Generator G)");
bool IsNonblocking(const Generator&, const Generator&);
SwigHelpEntry("Functions", "Composition", "Boolean IsNonblocking(+In+ Generator G1, +In+ Generator G2)");
bool IsNonblocking(const GeneratorVector&);
SwigHelpEntry("Functions", "Composition", "Boolean IsNonblocking(+In+ GeneratorVector G)");
#endif 

/* faudes-function "IsOmegaClosed" */
#if SwigModule == "SwigCoreFaudes"
bool IsOmegaClosed(const Generator&);
SwigHelpEntry("Functions", "Language misc", "Boolean IsOmegaClosed(+In+ Generator G)");
#endif 

/* faudes-function "IsOmegaTrim" */
#if SwigModule == "SwigCoreFaudes"
bool IsOmegaTrim(const Generator&);
SwigHelpEntry("Functions", "Reachability", "Boolean IsOmegaTrim(+In+ Generator G)");
#endif 

/* faudes-function "IsPrefixClosed" */
#if SwigModule == "SwigCoreFaudes"
bool IsPrefixClosed(const Generator&);
SwigHelpEntry("Functions", "Language misc", "Boolean IsPrefixClosed(+In+ Generator G)");
#endif 

/* faudes-function "IsTrim" */
#if SwigModule == "SwigCoreFaudes"
bool IsTrim(const Generator&);
SwigHelpEntry("Functions", "Reachability", "Boolean IsTrim(+In+ Generator G)");
#endif 

/* faudes-function "KleeneClosure" */
#if SwigModule == "SwigCoreFaudes"
void KleeneClosure(Generator&);
SwigHelpEntry("Functions", "Regular expressions", " KleeneClosure(+InOut+ Generator G)");
void KleeneClosure(const Generator&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " KleeneClosure(+In+ Generator GArg, +Out+ Generator GRes)");
#endif 

/* faudes-function "LanguageComplement" */
#if SwigModule == "SwigCoreFaudes"
void LanguageComplement(Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageComplement(+InOut+ Generator GPar)");
void LanguageComplement(const Generator&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageComplement(+In+ Generator GArg, +Out+ Generator GRes)");
void LanguageComplement(const Generator&, const EventSet&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageComplement(+In+ Generator GArg, +In+ EventSet Sigma, +Out+ Generator GRes)");
#endif 

/* faudes-function "LanguageConcatenate" */
#if SwigModule == "SwigCoreFaudes"
void LanguageConcatenate(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageConcatenate(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
#endif 

/* faudes-function "LanguageDifference" */
#if SwigModule == "SwigCoreFaudes"
void LanguageDifference(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageDifference(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
#endif 

/* faudes-function "LanguageDisjoint" */
#if SwigModule == "SwigCoreFaudes"
bool LanguageDisjoint(const Generator&, const Generator&);
SwigHelpEntry("Functions", "Language misc", "Boolean LanguageDisjoint(+In+ Generator G1, +In+ Generator G2)");
#endif 

/* faudes-function "LanguageEquality" */
#if SwigModule == "SwigCoreFaudes"
bool LanguageEquality(const Generator&, const Generator&);
SwigHelpEntry("Functions", "Language misc", "Boolean LanguageEquality(+In+ Generator G1, +In+ Generator G2)");
#endif 

/* faudes-function "LanguageInclusion" */
#if SwigModule == "SwigCoreFaudes"
bool LanguageInclusion(const Generator&, const Generator&);
SwigHelpEntry("Functions", "Language misc", "Boolean LanguageInclusion(+In+ Generator G1, +In+ Generator G2)");
#endif 

/* faudes-function "LanguageIntersection" */
#if SwigModule == "SwigCoreFaudes"
void LanguageIntersection(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageIntersection(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
void LanguageIntersection(const GeneratorVector&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageIntersection(+In+ GeneratorVector GVec, +Out+ Generator GRes)");
#endif 

/* faudes-function "LanguageUnion" */
#if SwigModule == "SwigCoreFaudes"
void LanguageUnion(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageUnion(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
void LanguageUnion(const GeneratorVector&, Generator&);
SwigHelpEntry("Functions", "Regular expressions", " LanguageUnion(+In+ GeneratorVector GVec, +Out+ Generator GRes)");
#endif 

/* faudes-function "MarkAllStates" */
#if SwigModule == "SwigCoreFaudes"
void MarkAllStates(Generator&);
SwigHelpEntry("", "", " MarkAllStates(+InOut+ Generator G)");
#endif 

/* faudes-function "OmegaClosure" */
#if SwigModule == "SwigCoreFaudes"
void OmegaClosure(Generator&);
SwigHelpEntry("Functions", "Language misc", " OmegaClosure(+InOut+ Generator GArg)");
#endif 

/* faudes-function "OmegaParallel" */
#if SwigModule == "SwigCoreFaudes"
%rename(OmegaParallel) aOmegaParallel;
void aOmegaParallel(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " OmegaParallel(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
#endif 

/* faudes-function "OmegaProduct" */
#if SwigModule == "SwigCoreFaudes"
%rename(OmegaProduct) aOmegaProduct;
void aOmegaProduct(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " OmegaProduct(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
#endif 

/* faudes-function "OmegaTrim" */
#if SwigModule == "SwigCoreFaudes"
void OmegaTrim(Generator&);
SwigHelpEntry("Functions", "Reachability", " OmegaTrim(+InOut+ Generator G)");
void OmegaTrim(const Generator&, Generator&);
SwigHelpEntry("Functions", "Reachability", " OmegaTrim(+In+ Generator GArg, +Out+ Generator GRes)");
#endif 

/* faudes-function "Parallel" */
#if SwigModule == "SwigCoreFaudes"
%rename(Parallel) aParallel;
void aParallel(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " Parallel(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
void aParallel(const GeneratorVector&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " Parallel(+In+ GeneratorVector GVec, +Out+ Generator GRes)");
void aParallel(const Generator&, const Generator&, ProductCompositionMap&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " Parallel(+In+ Generator G1, +In+ Generator G2, +Out+ ProductCompositionMap CompMap, +Out+ Generator GRes)");
#endif 

/* faudes-function "PrefixClosure" */
#if SwigModule == "SwigCoreFaudes"
void PrefixClosure(Generator&);
SwigHelpEntry("Functions", "Language misc", " PrefixClosure(+InOut+ Generator GArg)");
#endif 

/* faudes-function "Product" */
#if SwigModule == "SwigCoreFaudes"
%rename(Product) aProduct;
void aProduct(const Generator&, const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " Product(+In+ Generator G1, +In+ Generator G2, +Out+ Generator GRes)");
void aProduct(const Generator&, const Generator&, ProductCompositionMap&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " Product(+In+ Generator G1, +In+ Generator G2, +Out+ ProductCompositionMap CompMap, +Out+ Generator GRes)");
#endif 

/* faudes-function "Project" */
#if SwigModule == "SwigCoreFaudes"
%rename(Project) aProject;
void aProject(const Generator&, const EventSet&, Generator&);
SwigHelpEntry("Functions", "Language misc", " Project(+In+ Generator G, +In+ EventSet Sigma0, +Out+ Generator GRes)");
#endif 

/* faudes-function "SelfLoop" */
#if SwigModule == "SwigCoreFaudes"
void SelfLoop(Generator&, const EventSet&);
SwigHelpEntry("Functions", "Language misc", " SelfLoop(+InOut+ Generator G, +In+ EventSet Sigma)");
#endif 

/* faudes-function "StateMin" */
#if SwigModule == "SwigCoreFaudes"
%rename(StateMin) aStateMin;
void aStateMin(const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " StateMin(+In+ Generator GArg, +Out+ Generator GRes)");
void aStateMin(Generator&);
SwigHelpEntry("Functions", "Generator misc", " StateMin(+InOut+ Generator G)");
#endif 

/* faudes-function "Trim" */
#if SwigModule == "SwigCoreFaudes"
void Trim(Generator&);
SwigHelpEntry("Functions", "Reachability", " Trim(+InOut+ Generator G)");
void Trim(const Generator&, Generator&);
SwigHelpEntry("Functions", "Reachability", " Trim(+In+ Generator GArg, +Out+ Generator GRes)");
#endif 

/* faudes-function "UniqueInit" */
#if SwigModule == "SwigCoreFaudes"
void UniqueInit(Generator&);
SwigHelpEntry("Functions", "Generator misc", " UniqueInit(+InOut+ Generator G)");
void UniqueInit(const Generator&, Generator&);
SwigHelpEntry("Functions", "Generator misc", " UniqueInit(+In+ Generator G, +Out+ Generator GRes)");
#endif 

