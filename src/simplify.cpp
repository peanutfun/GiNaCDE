
/** @file simplify.cpp
 *
 *  Implementation of GiNaCDE's simplify function.
    It simplify algebraic expressions.*/



#include <ginac/ginac.h>
#include<cln/exception.h>
#include <math.h>
#include "utility.h"
#include "inifcns.h"
#include "simplify.h"
#include "solve.h"

using namespace std;
using namespace GiNaC;

size_t expandLevel = 3,addNumFrFactr = 20, largstNumsimp=100000000; /* expandLevel: To expand an expression upto a specified  degree (used in solvec::polySoluWtAutoDegSelct)
                                                                 // addNumFrFactr: To get factor upto specific number of "add" present (used in Factor in utility.cpp)
                                                                 // largstNumsimp: To get prime factors upto a specific number.(used in numSimplify::operator() )
                                                                 */

// A function to get all prime factors of a given number n
map<numeric,numeric> primeFactors(numeric n)
{
    map<numeric,numeric> primeNum;

        while (irem(n,2) == 0)
        {
            primeNum[2]=primeNum[2]+1;
                n = n/2;
        }

        // n must be odd at this point. So we can skip
        // one element (Note i = i +2)
        for (numeric i = 3; i <= GiNaC::sqrt(n); i = i+2)
        {
                // While i divides n, print i and divide n
                while (irem(n,i) == 0)
                {
            primeNum[i]=primeNum[i]+1;
                        n = n/i;
                }
        }

        // This condition is to handle the case when n
        // is a prime number greater than 2
        if (n > 2)
    {
        primeNum[n]=primeNum[n]+1;
    }

    return primeNum;
}
// Adding even, odd nature of sin, cos and (-5)^(7/2) = -125sqrt(5)*I
ex TrigArgSign_Complx::operator()(const ex & e)
{

    if (is_a<power>(e) && e.op(0).info(info_flags::negative)&& (2*e.op(1)).info(info_flags::odd))
        return I*sqrt(-e.op(0))*pow(-e.op(0),e.op(1) - _ex1_2);
    else if (is_ex_the_function(e, sin) || is_ex_the_function(e, tan) ||
             is_ex_the_function(e, csc) || is_ex_the_function(e, cot))
    {
        var_ = collect_common_factors(e.op(0)*symb_);
        var_ = var_.op(nops(var_)-1);
        if (is_a<numeric>(var_))
        {
            if (var_.info(info_flags::real) && var_ < _ex0 )
                return -e.subs(e.op(0) == -e.op(0), subs_options::algebraic).map(*this);
            else if (imag_part(var_) < _ex0)
                return -e.subs(e.op(0) == -e.op(0), subs_options::algebraic).map(*this);

        }
    }
    else if (is_ex_the_function(e, cos) || is_ex_the_function(e, sec))
    {
        var_ = collect_common_factors(e.op(0)*symb_);
        var_ = var_.op(nops(var_)-1);
        if (is_a<numeric>(var_))
        {
            if (var_.info(info_flags::real) && var_ < _ex0 )
                return e.subs(e.op(0) == -e.op(0), subs_options::algebraic).map(*this);
            else if (imag_part(var_) < _ex0)
                return e.subs(e.op(0) == -e.op(0), subs_options::algebraic).map(*this);

        }
    }

    return e.map(*this);
}

////////////                      simplifyc            //////////////
simplifyc Simplify;
Collect_common_factorsc Collect_common_factors;
numSimplify numSimplifye;
arguSimplify arguSimplifye;
expandinv expandinve;
fracNumericPowBasSubs fracNumericPowBasSubsE;
fracPowBasSubs fracPowBasSubsE;

int simplifyc::SetRules(int m)
{
    if(m == AlgSimp)
    {
        // (0)^a = 0
        AlgSimpRules[pow(0,wild(0))] = 0;
        AlgSimpRules[pow(wild(2),wild(0))*pow(wild(2),wild(1))] = pow(wild(2), wild(0)+wild(1));
        AlgSimpRules[wild(3)*pow(wild(2),wild(0))*pow(wild(2),wild(1))] = wild(3)*pow(wild(2), wild(0)+wild(1));
        AlgSimpRules[pow(pow(wild(2),wild(0)),wild(1))] = pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[wild(3)*pow(pow(wild(2),wild(0)),wild(1))] = wild(3)*pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[pow(wild(3)*pow(wild(2),wild(0)),wild(1))] = pow(wild(3),wild(1))*pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[wild(4)*pow(wild(3)*pow(wild(2),wild(0)),wild(1))] = wild(4)*pow(wild(3),wild(1))*pow(wild(2), wild(0)*wild(1));
        //AlgSimpRules[pow(wild(2),wild(0))*pow(wild(2),wild(1))] = pow(wild(2), wild(0)+wild(1));
        //AlgSimpRules[pow(wild(1),wild(0))*pow(wild(2),wild(0))] = pow(wild(1)*wild(2), wild(0));
        AlgSimpRules[pow(wild(1)*wild(2), wild(0))]=pow(wild(1),wild(0))*pow(wild(2),wild(0));
        AlgSimpRules[wild(3)*pow(wild(1)*wild(2), wild(0))]=wild(3)*pow(wild(1),wild(0))*pow(wild(2),wild(0));
        AlgSimpRules[pow(pow(wild(2),wild(0)),wild(1))] = pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[wild(3)*pow(pow(wild(2),wild(0)),wild(1))] = wild(3)*pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[pow(wild(3)*pow(wild(2),wild(0)),wild(1))] = pow(wild(3),wild(1))*pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[wild(4)*pow(wild(3)*pow(wild(2),wild(0)),wild(1))] = wild(4)*pow(wild(3),wild(1))*pow(wild(2), wild(0)*wild(1));
        AlgSimpRules[sqrt(wild(2)*pow(wild(0),wild(1)))] = pow(wild(2),_ex1_2)*pow(wild(0),wild(1)*_ex1_2);
        AlgSimpRules[wild(3)*sqrt(wild(2)*pow(wild(0),wild(1)))] = wild(3)*pow(wild(2),_ex1_2)*pow(wild(0),wild(1)*_ex1_2);

        // (a^2*b^3)^4 = a^8*b^12
       // AlgSimpRules[pow(pow(wild(0),wild(1))*pow(wild(2),wild(3)),wild(4))] = pow(wild(0),wild(1)*wild(4))*pow(wild(2),wild(3)*wild(4));
       // AlgSimpRules[wild(5)*pow(pow(wild(0),wild(1))*pow(wild(2),wild(3)),wild(4))] = wild(5)*pow(wild(0),wild(1)*wild(4))*pow(wild(2),wild(3)*wild(4));

        AlgSimpRules[exp(wild(0))*exp(wild(1))] = exp(wild(0)+wild(1));
        AlgSimpRules[wild(2)*exp(wild(0))*exp(wild(1))] = wild(2)*exp(wild(0)+wild(1));
        AlgSimpRules[pow(exp(wild(0)),wild(1))] = exp(wild(0)*wild(1));
        AlgSimpRules[wild(2)*pow(exp(wild(0)),wild(1))] = wild(2)*exp(wild(0)*wild(1));

        return 0;
    }

    else if (m == TrigSimp)
    {
        //rules = TrigSimp;
        TrigSimpRules1[wild(1)*pow(sin(wild(0)),2) + wild(1)*pow(cos(wild(0)),2)] = wild(1);
        TrigSimpRules1[wild(1)*pow(sec(wild(0)),2) - wild(1)*pow(tan(wild(0)),2)] = wild(1);
        TrigSimpRules1[wild(1)*pow(csc(wild(0)),2) - wild(1)*pow(cot(wild(0)),2)] = wild(1);
        TrigSimpRules1[pow(sin(wild(0)),2) + pow(cos(wild(0)),2)] = _ex1;
        TrigSimpRules1[pow(sec(wild(0)),2) - pow(tan(wild(0)),2)] = _ex1;
        TrigSimpRules1[pow(csc(wild(0)),2) - pow(cot(wild(0)),2)] = _ex1;
        TrigSimpRules2[tan(wild(0))] = sin(wild(0))/cos(wild(0));
        TrigSimpRules2[cot(wild(0))] = cos(wild(0))/sin(wild(0));
        TrigSimpRules2[csc(wild(0))] = 1/sin(wild(0));
        TrigSimpRules2[sec(wild(0))] = 1/cos(wild(0));
        TrigSimpRules3[sin(wild(0))/cos(wild(0))] = tan(wild(0));
        TrigSimpRules3[cos(wild(0))/sin(wild(0))] = cot(wild(0));
        TrigSimpRules4[1/sin(wild(0))] = csc(wild(0));
        TrigSimpRules4[1/cos(wild(0))] = sec(wild(0));

        return 0;
    }
    else if (m == TrigCombine)
    {
        //rules = TrigCombine;
        TrigCombineRules[sin(wild(0))*cos(wild(1))] = _ex1_2*(sin(wild(0)-wild(1))+sin(wild(0)+wild(1)));
        TrigCombineRules[sin(wild(0))*sin(wild(1))] = _ex1_2*(cos(wild(0)-wild(1))-cos(wild(0)+wild(1)));
        TrigCombineRules[pow(sin(wild(0)),2)] = _ex1_2*(1-cos(2*wild(0)));
        TrigCombineRules[cos(wild(0))*cos(wild(1))] = _ex1_2*(cos(wild(0)-wild(1))+cos(wild(0)+wild(1)));
        TrigCombineRules[pow(cos(wild(0)),2)] = _ex1_2*(1+cos(2*wild(0)));
        TrigCombineRules[sin(-wild(0)*wild(1))] = -sin(wild(0)*wild(1));
        TrigCombineRules[cos(-wild(0)*wild(1))] = cos(wild(0)*wild(1));

        return 0;
    }
    else if (m == logSimp)
    {
        //rules = logSimp;
        logSimpRules[log(pow(wild(0), wild(1)))] = wild(1)*log(wild(0));
        logSimpRules[log(wild(0)*wild(1))] = log(wild(0)) + log(wild(1));
        logSimpRules[log(exp(_ex1))] = _ex1;
    }
    return 0;
}

ex simplifyc::operator()(const ex& e, const int& rules,  const bool& isFracNegPowBaseGensymb)
{
    ex y=e;

    if(isFracNegPowBaseGensymb)
    {
        fracNumericPowBasSubsE.set();
        y = fracNumericPowBasSubsE(y);
    }

    if (rules == AlgSimp)
    {
        this->SetRules(AlgSimp);
        TrigArgSign_Complx trigarg;
        ex xprev;
        do
        {
            xprev = y;
            y = y.subs(AlgSimpRules, subs_options::algebraic);
            y = y.subs((lst){wild(2)*pow(wild(2),wild(1)) == pow(wild(2), _ex1+wild(1))},subs_options::subs_algebraic); //we apply this rule separately to increase speed.

            y = trigarg(y);
            numSimplifye.primefactrs.clear();
            y=(numSimplifye(y));
        } while(xprev != y);

        if(isFracNegPowBaseGensymb && !fracNumericPowBasSubsE.baseClt.empty())
        {
            y = (genSymbSubs(y,fracNumericPowBasSubsE.baseClt));
            do
            {
                xprev = y;
                y = y.subs(AlgSimpRules, subs_options::algebraic);
                y = y.subs((lst){wild(2)*pow(wild(2),wild(1)) == pow(wild(2), _ex1+wild(1))},subs_options::subs_algebraic); //we apply this rule separately to increase speed.
            } while(xprev != y);
        }

        return y;
    }
    else if (rules == TrigSimp)
    {
        y = this->operator()(y,AlgSimp);

        this->SetRules(TrigSimp);
        ex xprev;
        do
        {
            xprev = y;
            y = y.subs(TrigSimpRules4, subs_options::subs_algebraic).expand();
        } while(xprev != y);
        do
        {
            xprev = y;
            y = y.subs(TrigSimpRules3, subs_options::subs_algebraic).expand();
        } while(xprev != y);
        do
        {
            xprev = y;
            y = y.subs(TrigSimpRules2, subs_options::subs_algebraic).expand();
        } while(xprev != y);
        do
        {
            xprev = y;
            y = y.subs(TrigSimpRules1, subs_options::subs_algebraic).expand(expand_options::expand_function_args);
        } while(xprev != y);

        y = this->operator()(y,AlgSimp);
        return y;
    }
    else if (rules == TrigCombine)
    {
        y = this->operator()(y,AlgSimp);
        y = this->operator()(y,TrigSimp);

        this->SetRules(TrigCombine);
        ex xprev;
        do
        {
            xprev = y;
            y = y.subs(TrigCombineRules, subs_options::subs_algebraic).expand(expand_options::expand_function_args);
        } while(xprev != y);

        y = this->operator()(y,AlgSimp);
        return y;
    }
    else if (rules == logSimp)
    {
        y = this->operator()(y,AlgSimp);

        this->SetRules(logSimp);
        ex xprev;
        do
        {
            xprev = y;
            y = y.subs(logSimpRules, subs_options::subs_algebraic).expand(expand_options::expand_function_args);
        } while(xprev != y);

        y = this->operator()(y,AlgSimp);
        return y;
    }

    return e;
}

/// expanding terms containing inverse power
ex expandinv::operator()(const ex& e)
{
    if(is_a<mul>(e))
    {
        repls.clear();
        e.match(pow(wild(1), wild(0))*pow(wild(2), wild(0)), repls);
        e.match(wild(3)*pow(wild(1), wild(0))*pow(wild(2), wild(0)), repls);

        if(!repls.empty())
        {
            if((is_a<numeric>(repls[wild(0)]) && ex_to<numeric>(repls[wild(0)]).is_negative()) ||
                (nops(repls[wild(0)]) > 1 && is_a<numeric>(repls[wild(0)].op(nops(repls[wild(0)]) - 1))
                && ex_to<numeric>(repls[wild(0)].op(nops(repls[wild(0)]) - 1)).is_negative()))
            {
                if(repls[wild(3)] == _ex0)
                    return pow(expand(repls[wild(1)]*repls[wild(2)]), repls[wild(0)]);
                else
                    return repls[wild(3)]*pow(expand(repls[wild(1)]*repls[wild(2)]), repls[wild(0)]);
            }
        }
    }

    return e.map(*this);
}

/**doing factors of fractional power bases, function arguments. **/
ex arguSimplify::operator()(const ex &e)
{
    return (Simplify(Factor(e))).map(*this);
}

/** doing number simplify. **/
ex numSimplify::getPrimefactors(const ex &e, const ex &fractimes)
{
        primefactrs=primeFactors(ex_to<numeric>(e));

        if(!primefactrs.empty())
        {
            ex modiExpr=_ex1;
            for(auto itr = primefactrs.begin(); itr!=primefactrs.end();itr++)
            {
                modiExpr=modiExpr*pow(itr->first,itr->second*fractimes);
            }

            return modiExpr;
        }
        else
            return pow(e,fractimes);
}
/////////////////////////////////////////////////////////////
/// (6)^(3/2)=(3*2)^(3/2)=3^(3/2)*2^(3/2)
ex numSimplify::operator()(const ex& e)
{
    if(is_a<power>(e)&&is_a<numeric>((e.op(0)))
       &&is_a<numeric>((e.op(1))))
    {
        if( e.op(0).info(info_flags::positive) && e.op(1).info(info_flags::rational)
           &&((numer(e.op(0))<=largstNumsimp&&denom(e.op(0))<=largstNumsimp
           &&numer(e.op(1))<=largstNumsimp&&denom(e.op(1))<=largstNumsimp)))
        {
                return (numer(e.op(0))==_ex1?_ex1:getPrimefactors(numer(e.op(0)),e.op(1)))/(denom(e.op(0))==_ex1?_ex1:getPrimefactors(denom(e.op(0)),e.op(1)));
        }
        else if(e.op(0).info(info_flags::negative) && e.op(1).info(info_flags::rational))
        {
            if((-numer(e.op(0))<=largstNumsimp&&denom(e.op(0))<=largstNumsimp
           &&numer(e.op(1))<=largstNumsimp&&denom(e.op(1))<=largstNumsimp))
            {
                const ex temexpr = (numer(-e.op(0))==_ex1?_ex1:getPrimefactors(numer(-e.op(0)),e.op(1)))/(denom(e.op(0))==_ex1?_ex1:getPrimefactors(denom(e.op(0)),e.op(1)));
                return (pow(-1,(e.op(1)))*temexpr);
            }
            else
            {
                 return (pow(-1,(e.op(1)))*pow(-e.op(0),e.op(1)));
            }


        }

    }

    return e.map(*this);
}

/////////////////////////////////////////////////////////////
/** replacing the "pow" terms with created symbols, which have less degree than expandLevel and base is in add container.
    It is level 1 implementation of  powBaseSubsLessThanDeg. **/
ex powBaseSubsLessThanDegLvl_1::operator()(const ex& _e)
{
    if(is_a<power>(_e) && (is_a<add>(_e.op(0))) && (_e.op(1))>expandLevel)
    {
        if((!exprToSymMap.empty() && exprToSymMap.find(_e.op(0))==exprToSymMap.end())
            ||exprToSymMap.empty())
        {
            j=j+1;

            str = "powBaseSubs0_" + to_string(j);
            expr = reader(str);
            exprToSymMap[_e.op(0)]=expr;
        }

        if(!exprToSymMap.empty() && exprToSymMap.find(_e.op(0))!=exprToSymMap.end())
            return pow(exprToSymMap[_e.op(0)],_e.op(1));
    }

    return _e.map(*this);
}

/////////////////////////////////////////////////////////////
/** It is level 1 implementation of fracPowBasSubs. **/
ex fracPowBasSubsLvl_1::operator()(const ex& e)
{
    if(is_a<power>(e)&&denom(e.op(1))!=_ex1)
    {
        numer_denomClt = (e.op(0)).numer_denom();

        tem = Simplify(collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(0)))))/
                       collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(1))))));

        if((!baseCltLvl_1.empty() && baseCltLvl_1.find(pow(tem,pow(denom(e.op(1)),_ex_1)))==baseCltLvl_1.end())
            || baseCltLvl_1.empty())
        {
            j=j+1;

            str = "genSymb1_" + to_string(j);
            expr = reader(str);
            baseCltLvl_1[pow(tem,pow(denom(e.op(1)),_ex_1))]=expr;
        }

        if(!baseCltLvl_1.empty() && baseCltLvl_1.find(pow(tem,pow(denom(e.op(1)),_ex_1)))!=baseCltLvl_1.end())
            return pow(baseCltLvl_1[pow(tem,pow(denom(e.op(1)),_ex_1))],numer(e.op(1)));
    }

    return e.map(*this);
}

/////////////////////////////////////////////////////////////
/** replacing the "pow" terms with created symbols, which have less degree than expandLevel all type bases are included,
 * such as numeric, symbols, add container. This class has been used in Factor function before factoring.    **/
ex fracPowBasSubsFactor::operator()(const ex& e)
{
    if(is_a<power>(e)&&denom(e.op(1))!=_ex1)
    {

        numer_denomClt = (e.op(0)).numer_denom();

        tem = Simplify(collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(0)))))/
                       collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(1))))));

        if((!baseClt.empty() && baseClt.find(pow(tem,pow(denom(e.op(1)),_ex_1)))==baseClt.end())
            || baseClt.empty())
        {
            j=j+1;

            str = "genSymb1_" + to_string(j);
            expr = reader(str);
            baseClt[pow(tem,pow(denom(e.op(1)),_ex_1))]=expr;
        }

        if(!baseClt.empty() && baseClt.find(pow(tem,pow(denom(e.op(1)),_ex_1)))!=baseClt.end())
            return pow(baseClt[pow(tem,pow(denom(e.op(1)),_ex_1))],numer(e.op(1)));
    }

    return e.map(*this);
}

/////////////////////////////////////////////////////////////
/** replacing the "pow" terms with created symbols, which have degree less than expandLevel and base is in add container. **/
ex powBaseSubsLessThanDeg::operator()(const ex& _e)
{
    if(is_a<power>(_e) && (is_a<add>(_e.op(0))) && _e.op(1).info(info_flags::integer) &&
        ((_e.op(1))>expandLevel||(-_e.op(1))>expandLevel))
    {
        Lvl_1.set();
        numer_denomClt = Lvl_1(_e.op(0)).numer_denom();

        tem = Simplify(collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(0)))))/
                       collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(1))))));

        if(!Lvl_1.exprToSymMap.empty())
            tem = Simplify(genSymbSubs(tem,Lvl_1.exprToSymMap));

        if((!exprToSymMap.empty() && exprToSymMap.find(tem)==exprToSymMap.end())
            ||exprToSymMap.empty())
        {
            j=j+1;

            str = "powBaseSubs2_" + to_string(j);
            expr = reader(str);
            exprToSymMap[tem]=expr;
        }

        if(!exprToSymMap.empty() && exprToSymMap.find(tem)!=exprToSymMap.end())
            return pow(exprToSymMap[tem],_e.op(1));
    }

    return _e.map(*this);
}

////////////////////////////////////////////////////////////////
/** replacing base (works on symbol and numerics bases) of fractional or negative power with generated symbols.  **/
ex fracNumericPowBasSubs::operator()(const ex& e)
{
    if(is_a<power>(e) && (is_a<numeric>(e.op(0))||is_a<symbol>(e.op(0)))&&
        ((e.op(1)).info(info_flags::negative)||denom(e.op(1))!=_ex1))
    {
        if((!baseClt.empty() && baseClt.find(e.op(1))==baseClt.end())
            || baseClt.empty())
        {
            j=j+1;

            str = "genSymb3_" + to_string(j);
            expr = reader(str);
            baseClt[e.op(1)]=expr;
        }

        if(!baseClt.empty() && baseClt.find(e.op(1))!=baseClt.end())
        {
            return pow(e.op(0),baseClt[e.op(1)]);
        }

    }

     return e.map(*this);
}

/** replacing base of fractional power with generated symbols. This simplifies all fractional power with base expressions.  **/
ex fracPowBasSubs::operator()(const ex& e)
{
    if(is_a<power>(e)&&denom(e.op(1))!=_ex1)
    {

        numer_denomClt = (e.op(0)).numer_denom();

        tem = Simplify(collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(0)),AlgSimp,false)))/
                       collect_common_factors(Factor(Simplify(expand(numer_denomClt.op(1)),AlgSimp,false))),AlgSimp,false);
        tem=Lvl_1(tem);
        if(!Lvl_1.baseCltLvl_1.empty())
        {
            ex temtemexpr_;
            for(auto it = Lvl_1.baseCltLvl_1.begin(); it != Lvl_1.baseCltLvl_1.end(); it++) // coefficients of each generated symbol are simplified separately
            {
                if(tem.is_polynomial((*it).second))
                {
                    temtemexpr_ = _ex0;
                    for(int i = 0; i <=degree(tem,(*it).second); i++)
                    {
                        numer_denomClt = (tem.coeff((*it).second,i)).numer_denom();
                        temtemexpr_ = temtemexpr_+ pow((*it).second,i)*Simplify((is_a<numeric>(numer_denomClt.op(0))?numer_denomClt.op(0):collect_common_factors((Factor(Simplify((numer_denomClt.op(0)),AlgSimp,false)))))/
                                                                                        (is_a<numeric>(numer_denomClt.op(1))?numer_denomClt.op(1):collect_common_factors((Factor(Simplify((numer_denomClt.op(1)),AlgSimp,false))))),AlgSimp,false);
                    }
                    tem = temtemexpr_;
                }
                else
                {
                    numer_denomClt = tem.numer_denom();
                    tem = Simplify((is_a<numeric>(numer_denomClt.op(0))?numer_denomClt.op(0):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(0)),AlgSimp,false)))))/
                                            (is_a<numeric>(numer_denomClt.op(1))?numer_denomClt.op(1):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(1)),AlgSimp,false))))),AlgSimp,false);
                }
            }

            tem = Simplify(genSymbSubs(tem,Lvl_1.baseCltLvl_1),AlgSimp,false);
            Lvl_1.set();
        }

        if((!baseClt.empty() && baseClt.find(pow(tem,pow(denom(e.op(1)),_ex_1)))==baseClt.end())
           || baseClt.empty())
        {
            j=j+1;

            str = "genSymb4_" + to_string(j);
            expr = reader(str);
            baseClt[pow(tem,pow(denom(e.op(1)),_ex_1))]=expr;
        }

        if(!baseClt.empty() && baseClt.find(pow(tem,pow(denom(e.op(1)),_ex_1)))!=baseClt.end())
            return pow(baseClt[pow(tem,pow(denom(e.op(1)),_ex_1))],numer(e.op(1)));

    }

    return e.map(*this);
};

////////////////////////////////////////////////////////////
/// This collect all common factors including numerical numbers.
///
ex Collect_common_factorsc::operator()(const ex& _e)
{
    if(is_a<add>(_e))
    {
        temex = collect_common_factors(factSymb_*_e);
        temex2 = _ex1;

        for (size_t i=0;i<nops(temex);i++)
        {
            if(!is_a<add>(temex.op(i)))
                temex2=temex2*temex.op(i);
        }

        if((temex2) != factSymb_)
            return (temex); // N.B: don't map when an extra expression (here factSymb_) is included
        else                // in original expression.
            return (_e.map(*this));
    }

    return _e.map(*this);
}

////////////////////////////////////////////////////////////

ex simplify(const ex& expr_, int rules)
{
    if(expr_==_ex0)
        return expr_;


    try
    {
        ex temexpr_ = expr_, xprev;
        ex numer_denomClt;
        exmap temBaseClt,temexprToSymMap;   //It stores the values of variables in subclasses of map_function.
                                            //All the subclasses of same base class map_function, when works at the same time,
                                            //the variables of subclasses are influenced by each other. To remove this problem, variables in subclasses
                                            // are stored by other variables before use of second subclasses. Here the subclasses fracPowBasSubsE,
                                            // and fracPowBasSubsFactor (used in Factor) has been used at the same time.


        if(expr_.info(info_flags::relation)) // including relation operator (==)
            temexpr_ = expr_.rhs();

        temexpr_ = temexpr_.subs(factSymb_==_ex1);

        temexpr_=Simplify(temexpr_,rules,false);

        if(is_a<numeric>(temexpr_)) // This return numeric expressions without more simplify.
        {
            if(expr_.info(info_flags::relation))
                return expr_.lhs()==(temexpr_);
            return  temexpr_;
        }

        powBaseSubsLessThanDeg baseSubs(0); // At first all expressions having degree less than expandLevel is expanded
        temexpr_ = baseSubs(temexpr_);      // and expressions having degree greater then expandLevel is replaced by
        temexpr_=Simplify(temexpr_,rules,false);  // generated symbols.

        temexprToSymMap = baseSubs.exprToSymMap;

        fracNumericPowBasSubsE.set();
        temexpr_=fracNumericPowBasSubsE(temexpr_);

        if(!fracNumericPowBasSubsE.baseClt.empty())
        {
            temBaseClt = fracNumericPowBasSubsE.baseClt;

            numer_denomClt = temexpr_.numer_denom();
            temexpr_ = Simplify((is_a<numeric>(numer_denomClt.op(0))?numer_denomClt.op(0):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(0)),rules,false)))))/
                                    (is_a<numeric>(numer_denomClt.op(1))?numer_denomClt.op(1):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(1)),rules,false))))),rules,false);
            temexpr_ = Simplify(genSymbSubs(temexpr_,temBaseClt),rules,false);
        }


        do // This collect all common factors including numerical numbers
        {
            xprev =temexpr_;
            temexpr_ = Collect_common_factors(temexpr_);
            temexpr_ = temexpr_.subs(factSymb_==_ex1);
            temexpr_ = Simplify(temexpr_,rules,false);

        } while(xprev != temexpr_);
        temexpr_ = temexpr_.subs(factSymb_==_ex1);


        fracPowBasSubsE.set();
        temexpr_=fracPowBasSubsE(temexpr_);
        if(!fracPowBasSubsE.baseClt.empty())
        {
            ex temtemexpr_;
            temBaseClt = fracPowBasSubsE.baseClt;
            for(auto it = temBaseClt.begin(); it != temBaseClt.end(); it++)
            {
                if(temexpr_.is_polynomial((*it).second))
                {
                    temtemexpr_ = _ex0;
                    temexpr_ = expand(temexpr_);
                    for(int i = 0; i <=degree(temexpr_,(*it).second); i++)
                    {
                        numer_denomClt = (temexpr_.coeff((*it).second,i)).numer_denom();
                        temtemexpr_ = temtemexpr_+ pow((*it).second,i)*Simplify((is_a<numeric>(numer_denomClt.op(0))?numer_denomClt.op(0):collect_common_factors((Factor(Simplify((numer_denomClt.op(0)),rules,false)))))/
                                                                 (is_a<numeric>(numer_denomClt.op(1))?numer_denomClt.op(1):collect_common_factors((Factor(Simplify((numer_denomClt.op(1)),rules,false))))),rules,false);
                    }
                    temexpr_ = temtemexpr_;
                }
                else
                {
                    numer_denomClt = temexpr_.numer_denom();
                    temexpr_ = Simplify((is_a<numeric>(numer_denomClt.op(0))?numer_denomClt.op(0):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(0)),rules,false)))))/
                                            (is_a<numeric>(numer_denomClt.op(1))?numer_denomClt.op(1):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(1)),rules,false))))),rules,false);
                }
            }

            temexpr_ = Simplify(genSymbSubs(temexpr_,temBaseClt),rules,false);
        }

        numer_denomClt = temexpr_.numer_denom();
        temexpr_ = Simplify((is_a<numeric>(numer_denomClt.op(0))?numer_denomClt.op(0):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(0)),rules,false)))))/
                            (is_a<numeric>(numer_denomClt.op(1))?numer_denomClt.op(1):collect_common_factors((Factor(Simplify(expand(numer_denomClt.op(1)),rules,false))))),rules,false);


        temexpr_ = Simplify(genSymbSubs(temexpr_,temexprToSymMap),rules,false);

        if(expr_.info(info_flags::relation))
           return expr_.lhs()==(temexpr_);

        return (temexpr_);
    }

    catch(GiNaC::pole_error){return expr_.subs(factSymb_==_ex1);}
    catch (cln::runtime_exception){return expr_.subs(factSymb_==_ex1);}

    catch(std::invalid_argument){return expr_.subs(factSymb_==_ex1);}
    catch(std::out_of_range){return expr_.subs(factSymb_==_ex1);}

    return (expr_);
}
ex simplifyRecur(const ex& expr_, int rules)
{
    ex prev = expr_, curr = expr_;

    do
    {
        prev = curr;
        curr = simplify(prev, rules);
    }while(prev!=curr);

    return  curr;
}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


