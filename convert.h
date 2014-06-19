// -*- C++ -*-
/*
 * File:   convert.h
 * Author: annie
 *
 * Created on 19 Июнь 2014 г., 14:28
 */

#ifndef CONVERT_H
#define	CONVERT_H


#include <string>
#include <locale>
#include <cwchar>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "utils.h"
#include "codecvt_cp1251.h"


namespace mas
{
    void convert(const std::string & inputPath, const std::string & outputPath);
} // namespace mas



#endif	/* CONVERT_H */

