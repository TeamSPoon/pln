/*
 * opencog/reasoning/pln/PLNModule.cc
 *
 * Copyright (C) 2002-2007 Novamente LLC
 * Copyright (C) 2008 by Singularity Institute for Artificial Intelligence
 * All Rights Reserved
 *
 * Written by Jared Wigmore <jared.wigmore@gmail.com>
 * Contains adapted code from PLNShell.cc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "PLNModule.h"

#include "PLN.h"
#include "rules/Rules.h"

#define BackInferenceTreeRootT BITNodeRoot

//#include "PTLEvaluator.h"

#include "rules/RuleProvider.h"
#include "AtomSpaceWrapper.h"
#include "BackInferenceTreeNode.h"
#include <opencog/util/Logger.h>
#include <opencog/atomspace/utils.h>
// #include <opencog/guile/SchemeEval.h>

#include <boost/foreach.hpp>
#include <stdlib.h>
#include <time.h>
#include <sstream>

#include "TestTargets.h"

using namespace opencog;
using namespace reasoning;

DECLARE_MODULE(PLNModule)

#ifdef _MSC_VER
#pragma warning(disable : 4312)
#endif // _MSC_VER

//! debug level, @todo replaced by opencog log system
extern int currentDebugLevel;

namespace haxx
{
    extern bool AllowFW_VARIABLENODESinCore;
    extern bool ArchiveTheorems;
    extern bool printRealAtoms;
//    extern reasoning::iAtomSpaceWrapper* defaultAtomSpaceWrapper;
}

namespace reasoning
{
    extern bool RECORD_TRAILS;
}

namespace test
{
    extern FILE *logfile;
//    int _test_count = 0;
    bool debugger_control = false;
}

//void initTestEnv();
//void Init();
std::string RunCommand(std::list<std::string> args);

PLNModule::PLNModule() : Module()
{
    logger().info("[PLNModule] constructor");
	do_pln_register();
}

PLNModule::~PLNModule() {
    logger().info("[PLNModule] destructor");
    do_pln_unregister();
}

// state variables for running multiple PLNShell commands.
Btr<BackInferenceTreeRootT> Bstate;
BackInferenceTreeRootT* temp_state, *state;

void PLNModule::init()
{
    logger().info("[PLNModule] init");
//    CogServer& cogserver = static_cast<CogServer&>(server());

// TODO this should be moved to a separate method so it can
// also be accessed from PLNUTest
//	initTestEnv();
    RECORD_TRAILS = true;

    currentDebugLevel=100;
	
	// Make sure that the ASW is initialized on module load
	iAtomSpaceWrapper* asw = ASW();
	
//    Init();
    #if LOCAL_ATW
//    haxx::defaultAtomSpaceWrapper = &reasoning::LocalATW::getInstance();
    ((LocalATW*)asw)->SetCapacity(10000);
    #endif  
    
    #if LOG_ON_FILE
     test::logfile=fopen("pln.log","wt");
     cout << "LOGGING TO FILE pln.log!\n";
    #endif

    haxx::printRealAtoms = true;
    haxx::ArchiveTheorems = false;

    // no longer done at module load - it would be inappropriate
    // for contexts other than testing PLN
    /*initTests();*
    Bstate = Btr<BackInferenceTreeRootT>(new BITNodeRoot(tests[0],
        new DefaultVariableRuleProvider));
    printf("BITNodeRoot init ok\n");
    temp_state = Bstate.get();
    state = Bstate.get();*/
}

std::string PLNModule::do_pln(Request *dummy, std::list<std::string> args)
{
/*	if (args.size() != 3)
		return "sql-load: Wrong num args";*/

/*	std::string dbname   = args.front(); args.pop_front();
	std::string username = args.front(); args.pop_front();
	std::string auth	   = args.front(); args.pop_front();*/

//    initTestEnv(args);

    std::string output = RunCommand(args);

	return output;
}

#if 0
void initTestEnv()
{
    try {
        puts("Initializing PLN test env...");

        RECORD_TRAILS = true;
        haxx::printRealAtoms = true;

        currentDebugLevel=100;

//        LOG(2, "Creating AtomSpaceWrappers...");
        
/*
#if LOCAL_ATW
        haxx::defaultAtomSpaceWrapper = &LocalATW::getInstance();
#else
        DirectATW::getInstance();
        haxx::defaultAtomSpaceWrapper = &NormalizingATW::getInstance();
#endif*/
//        AtomSpaceWrapper& atw = *GET_ATW;
        
#if 0 //Loading Osama or set axioms here.

        haxx::ArchiveTheorems = true;
     
    //  bool axioms_ok = atw.loadAxioms("bigdemo.xml");
    //  bool axioms_ok = atw.loadAxioms("inverse_binding.xml");
    //  bool axioms_ok = atw.loadAxioms("fetch10.xml");
    //  bool axioms_ok = atw.loadAxioms("mediumdemo.xml");
        bool axioms_ok = atw.loadAxioms("smalldemo.xml");
    //  bool axioms_ok = atw.loadAxioms("smalldemo28.xml");
    //  bool axioms_ok = atw.loadAxioms("smalldemo28b.xml");
    //  bool axioms_ok = atw.loadAxioms("smalldemo8.xml");
    //  bool axioms_ok = atw.loadAxioms("smalldemo8b.xml");  
    //  bool axioms_ok = atw.loadAxioms("smalldemo8c.xml");
    //  bool axioms_ok = atw.loadAxioms("AnotBdemo.xml");
    //  bool axioms_ok = atw.loadAxioms("fetchdemo5.xml");
    //  bool axioms_ok = atw.loadAxioms("fetchdemo.xml");
    //  bool axioms_ok = atw.loadAxioms("woademo.xml");
        assert(axioms_ok);

        haxx::ArchiveTheorems = false;
#endif
        logger().debug("PTL Initialized.");
    }
    catch(std::string s) {
        logger().error("at root level while RunLoop initializing.");
    }
    catch(PLNexception e)
    {
        logger().error("at root level while RunLoop initializing.");
    }
    catch(...)
    {
        logger().error("Unknown exception at root level while RunLoop initializing. ");
        cout << "Unknown exception at root level while RunLoop initializing. "<< endl;
    }

    try
    {
        //RunPLNTests();
//        ThePLNShell.Launch();

        return;
    }
    catch(string s) {
        printf("Exception in initTestEnv(): %s\n", s.c_str()); }
    catch(boost::bad_get bg) {
        printf("Bad boost::get in initTestEnv(): %s\n", bg.what()); }
    catch(...) {
        puts("Exception in initTestEnv()."); }
  
    getc(stdin);
}
#endif

#if 0
void Init()
{
    #if LOCAL_ATW
//    haxx::defaultAtomSpaceWrapper = &reasoning::LocalATW::getInstance();
    ((LocalATW*)ASW())->SetCapacity(10000);
    #endif  
    
    #if LOG_ON_FILE
     test::logfile=fopen("pln.log","wt");
     cout << "LOGGING TO FILE pln.log!\n";
    #endif

    haxx::printRealAtoms = true;
    haxx::ArchiveTheorems = false;
}
#endif

template <typename T>
T input(T& a, std::list<std::string> args)
{
//    T a;
    std::stringstream ss;
    ss << args.front();
    args.pop_front();
    ss >> a;
    
    return a;
}

std::string RunCommand(std::list<std::string> args)
{
    AtomSpaceWrapper* atw = GET_ATW;

/*  vector<Vertex> targs, targs2;
    targs.push_back(mva((Handle)INHERITANCE_LINK,
                    NewNode(CONCEPT_NODE, "A"),
                    NewNode(CONCEPT_NODE, "B")
            ));
    targs.push_back(mva((Handle)INHERITANCE_LINK,
                    NewNode(CONCEPT_NODE, "B"),
                    NewNode(CONCEPT_NODE, "C")
            ));
    targs2.push_back(mva((Handle)INHERITANCE_LINK,
                    NewNode(CONCEPT_NODE, "A"),
                    NewNode(CONCEPT_NODE, "D")
            ));
    targs2.push_back(mva((Handle)INHERITANCE_LINK,
                    NewNode(CONCEPT_NODE, "B"),
                    NewNode(CONCEPT_NODE, "C")
            ));
*/
//  assert(RuleRepository::Instance().rule[Deduction]->validate(targs));
//  assert(!RuleRepository::Instance().rule[Deduction]->validate(targs));

    std::stringstream ss;

    try {
        printf("Root = %ld\n", (long)state);
        
//    while (1)
//    {
//        puts("(h = help):");

        int a1T, a2T, bT, tempi=0;
        string a10, a11, a20, a21, b1, b2;
        vtree avt1, avt2, bvt;
        int qrule=NULL;
        Rule::MPs rule_args;
        bindingsT new_bindings;

        char c = args.front()[0];
        args.pop_front();
//        char temps[1000];
        std::string temps;
        long h=0, h2=0;
        int j;
        int test_i=0;
        int s_i=0;
        bool axioms_ok;
        boost::shared_ptr<set<Handle> > ts;
        Vertex v;
        Handle eh=Handle::UNDEFINED;
        bool using_root = true;
        
        #if LOG_ON_FILE
            save_log();
        #endif
        
        haxx::AllowFW_VARIABLENODESinCore = true; //false;
        
        switch (c)
        {
            case 'm':
                //printf("%d\n", (int)tests.size()); break;
                ss << (int)tests.size() << std::endl;
            case 'd':
#if LOCAL_ATW
            ((LocalATW*)atw)->DumpCore(CONCEPT_NODE);
#else
            input(h, args);
            ts = atw->getHandleSet((Type)h,"");
            foreach(Handle ti, *ts)
            {
                if (atw->getTV(ti).isNullTv())
                {
                    puts("NULL TV !!!");
                    // TODO
                    getc(stdin);
                }
                printTree(ti,0,0);
            }           
#endif
                 break;
            case 'k': state->loopCheck(); break;
            case 'D': test::debugger_control = (test::debugger_control?false:true);
/*                      for (int zz=0;zz<1000;zz++)
                            RuleRepository::Instance().rule[ForAll]->o2iMeta(
                                meta(new vtree(mva((Handle)EVALUATION_LINK,
                                NewNode(PREDICATE_NODE, "friendOf"),
                                mva((Handle)LIST_LINK,
                                NewNode(CONCEPT_NODE, "Britney"),
                                NewNode(CONCEPT_NODE, "Amir")
                                )))));

                            //atw->getHandle(NODE, "temmpo");
                        puts("...");*/
                        break;
            case 'a': input(h, args); state->extract_plan((Handle)h); break;
            case 'A': input(h, args);
                            ((BackInferenceTreeRootT*)h)->printArgs();
                            
                            break;
            case 'U': input(h, args);
                        //((BackInferenceTreeRootT*)state->children[0].begin()->prover)
                        state->print_parents((BITNode*)h);
                        break;
            case 'b': input(h, args);
                        cprintf(-10, "Target:\n");
                        ((BackInferenceTreeRootT*)h)->printTarget();
                        cprintf(-10, "Results:\n");
                        ((BackInferenceTreeRootT*)h)->printResults();

                        cprintf(0, "parent arg# %d\n", ((BackInferenceTreeRootT*)h)->getParents().begin()->parent_arg_i);

                         break;
/*          case 'B': input(h, args); cprintf(0, "Node has results & bindings:\n");
                        foreach(const BoundVertex& bv, *((BackInferenceTreeRootT*)h)->direct_results)
                        {
                            cprintf(0,"[%d]\n", v2h(bv.value));
                            if (bv.bindings)
                                foreach(hpair phh, *bv.bindings)
                                {
                                    printTree(phh.first,0,0);
                                    cprintf(0,"=>");
                                    printTree(phh.second,0,0);
                                }
                            }
                         break;*/
            // TODO deal with arguments to this thing
            case 'H': 
                //input(h, args); cprintf(0,"Origin Node #%d\n", state->hsource[(Handle)h]); break;
                        fflush(stdin);
                        input(bT, args); input(b1, args); cin >> b2;
                        cin >> a1T; cin >> a10; cin >> a11;
                        cin >> a2T; cin >> a20; cin >> a21;
                        puts("Enter Rule #: ");
                        cin >> qrule;

                        bvt = (mva((Handle)bT, NewNode(CONCEPT_NODE, b1), NewNode(CONCEPT_NODE, b2)));
                        avt1 = (mva((Handle)a1T, NewNode(CONCEPT_NODE, a10), NewNode(CONCEPT_NODE, a11)));
                        avt2 = (mva((Handle)a2T, NewNode(CONCEPT_NODE, a20), NewNode(CONCEPT_NODE, a21)));

                        rawPrint(bvt, bvt.begin(), -2);
                        rawPrint(avt1, avt1.begin(), -2);
                        rawPrint(avt2, avt2.begin(), -2);

                        rule_args.clear();
                        rule_args.push_back(BBvtree(new BoundVTree(avt1)));
                        if (a2T)
                            rule_args.push_back(BBvtree(new BoundVTree(avt2)));
                        else
                            puts("Passing 1 arg.");

                        printf("BITNode %ld.", (long) state->findNode((Rule*)qrule, meta(new vtree(bvt)), rule_args, new_bindings));

                        break;
            case 'F':   tempi = currentDebugLevel;
                        currentDebugLevel = 10;
                        state->printFitnessPool(); break;
                        currentDebugLevel = tempi;
            case 'W':   if (using_root)
                            {
                                temp_state = state;
                                input(h, args);
                                state = (BackInferenceTreeRootT*)h;

                                using_root = false;
                            }
                            else
                            {
                                state = temp_state;

                                using_root = true;
                            }
                            break;
            case '=': input(h, args); input(h2, args); 
                      cprintf(0, ((BackInferenceTreeRootT*)h)->eq((BackInferenceTreeRootT*)h2) ? "EQ\n" : "IN-EQ\n"); break;
/*                      ss << ( ((BackInferenceTreeRootT*)h)->eq((BackInferenceTreeRootT*)h2) ?
                                "EQ" : "IN-EQ" ) << std::endl;*/
            case 'p': input(h, args); printTree((Handle)h,0,0); break;
            case 'e':   try { state->evaluate(); }
                        catch(string s) { cprintf(0,s.c_str()); }
                        break;
            
            case 'E': input(h, args);
                        if (h == 0)
                            h = (long)state;
//                          h = (int)state->children[0].begin()->prover;
                        ((BackInferenceTreeRootT*)h)->printResults();
/*                      foreach(const set<BoundVertex>& eval_res_set, ((BackInferenceTreeRootT*)h)->GetEvalResults())
                            foreach(const BoundVertex& eval_res, eval_res_set)
                                printTree(v2h(eval_res.value),0,-10);*/
                        break;
            case 'i': input(h, args);
                        ((BackInferenceTreeRootT*)h)->expandNextLevel();
/*                      foreach(const parent_link& p, ((BackInferenceTreeRootT*)h)->GetParents())
                            p.link->removeIfFailure((BackInferenceTreeRootT*)h);*/
                        break;

            case 'n': state->expandNextLevel(); break;
            case 't': input(h, args); state->printTrail((Handle)h); break;
            case 'f': state->expandFittest(); break;

            case 'P': input(h, args);
                        if (h == 0)
                            h = (long) state->children[0].begin()->prover;
                        ((BackInferenceTreeRootT*)h)->print(); break;
            case 'O':   input(h, args);
//                      ((BITNode*)h)->PrintUsers();
                        foreach(const parent_link<BITNode>& p, ((BITNode*)h)->getParents())
                            cprintf(-10,"User Node = %lu\n", (ulong) p.link);
                        break;
//          case 'l': cprintf(0,"%d\n", state->exec_pool.size()); break;
                
            case 's':   input(h, args); //Give max nr of steps that we can take.
                        j = (long) h;
                
                        //printf("\nTemporarily killing the log with level -3.\n");
                        //tempi = currentDebugLevel;
                        //currentDebugLevel = -3;
            
                        state->infer(j, 0.000001f, 0.01f);
                        state->printResults();
                        printf("\n%ld $ remaining.\n", h);
            
                        //currentDebugLevel = tempi;

                        break;
                                        
            case 'S':   s_i=0;
                        input(h, args);
            
                        for (int k=0;k<h;k++)
                            state->expandFittest();

                        break;
        
            case 'r': input(test_i, args);
                        Bstate.reset(new BITNodeRoot(tests[test_i], new DefaultVariableRuleProvider));
                        state = Bstate.get();
                        using_root = true;

                        cprintf(0,"Now evaluating: ");
                        rawPrint(*tests[test_i],tests[test_i]->begin(),0);
                        cprintf(0,"\n");
                        
//                      state->expandNextLevel();
                        break;
            case 'x': //puts("Give the XML input file name: "); 
                        input(temps, args);
                        haxx::ArchiveTheorems = true; 
                        atw->reset();
                        axioms_ok = atw->loadAxioms(temps);
                        haxx::ArchiveTheorems = false;
                        puts(axioms_ok ? "Input file was loaded." : "Input file was corrupt.");
            
                        puts("Next you MUST (re)load a target atom with r command! Otherwise things will break.\n");
                        // have to recreate the target vtrees to ensure that the
                        // handles are correct after reloading axioms.
                        initTests();

                        break;
            case 'c': RECORD_TRAILS = !RECORD_TRAILS;
                        //cprintf(0, "RECORD_TRAILS %s\n", (RECORD_TRAILS?"ON":"OFF"));
                        ss << "RECORD_TRAILS " << (RECORD_TRAILS?"ON":"OFF") << "\n";
                        break;
            case '-': input(h, args); currentDebugLevel = -(int)h; break;

/*            case 'R':
                int testdone;
                atw->testAtomSpaceWrapper();
                input(testdone, args);
//                fw_beta();
                break;*/
            case 'Y':
                // Temporary for loading data via telnet
                //cout << "running server loop" << endl;
                //static_cast<CogServer&>(server()).unitTestServerLoop(1);
                //cout << "finished server loop" << endl;
                break;
            default: c = c-'0';
                    if (c >= 0 && c <= 10)
                        currentDebugLevel = c;
                    break;
        }
//    }
    } catch( exception& e )
    {
        cout << endl << "Exception: "
             << e.what() << endl;
    }
    
    return ss.str();
}
