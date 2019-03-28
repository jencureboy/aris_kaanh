﻿#ifndef ROKAE_CPLAN_H_
#define ROKAE_CPLAN_H_

#include <aris.hpp>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <vector>
#include <memory>
#include <atomic>
#include <string> 
#include <algorithm>


class MoveCircle : public aris::plan::Plan
{
public:
    auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
    auto virtual executeRT(aris::plan::PlanTarget &target)->int;
    //auto virtual collectNrt(aris::plan::PlanTarget &target)->void;

    explicit MoveCircle(const std::string &name = "MoveCircle");
};


class MoveTroute : public aris::plan::Plan
{
public:
    auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
    auto virtual executeRT(aris::plan::PlanTarget &target)->int;
    //auto virtual collectNrt(aris::plan::PlanTarget &target)->void;

    explicit MoveTroute(const std::string &name = "MoveTroute");
};


class MoveFile : public aris::plan::Plan
{
public:
    auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
    auto virtual executeRT(aris::plan::PlanTarget &target)->int;
    //auto virtual collectNrt(aris::plan::PlanTarget &target)->void;

    explicit MoveFile(const std::string &name = "MoveFile");
};

class RemoveFile : public aris::plan::Plan
{
public:
	auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
	auto virtual executeRT(aris::plan::PlanTarget &target)->int;

	explicit RemoveFile(const std::string &name = "RemoveFile");
};

auto load_pq2(aris::Size count, aris::Size &start_count)->std::array<double, 14>;

auto load_pq5()->void;

auto load_pq7(aris::Size count, aris::Size &start_count)->std::array<double, 14>;

class MoveinModel : public aris::plan::Plan
{
public:
	auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
	auto virtual executeRT(aris::plan::PlanTarget &target)->int;

	explicit MoveinModel(const std::string &name = "MoveinModel");
};


class FMovePath : public aris::plan::Plan
{
public:
	auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
	explicit FMovePath(const std::string &name = "FMovePath_plan");
};

class MoveLPolish : public aris::plan::Plan
{
public:
	auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
	explicit MoveLPolish(const std::string &name = "MoveLPolish_plan");
};

class OpenFile : public aris::plan::Plan
{
public:
	auto virtual prepairNrt(const std::map<std::string, std::string> &params, aris::plan::PlanTarget &target)->void;
	explicit OpenFile(const std::string &name = "OpenFile");
};

#endif
