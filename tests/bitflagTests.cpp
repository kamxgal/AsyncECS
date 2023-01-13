/*
 * AsyncECS
 * Copyright (c) 2018 kamxgal Kamil Galant kamil.galant@gmail.com
 *
 * MIT licence
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <gtest/gtest.h>

#include "bitflag.h"

TEST(BitflagShould, SetAndUnsetProperFlag)
{
    bitflag f(4);
    EXPECT_EQ("0000", f.str());
    f.set(3, true);
    EXPECT_EQ("0001", f.str());
    f.set(0, true);
    EXPECT_EQ("1001", f.str());
    f.set(0, false);
    EXPECT_EQ("0001", f.str());
    f.set(3, false);
    EXPECT_EQ("0000", f.str());
}

TEST(BitflagShould, SetAndUnsetProperFlagAfterResize)
{
    bitflag f(4);
    f.set(3, true);
    EXPECT_EQ("0001", f.str());

    f.resize(11);
    EXPECT_EQ("00010000000", f.str());
    f.set(3, false);
    EXPECT_EQ("00000000000", f.str());
    f.set(9, true);
    EXPECT_EQ("00000000010", f.str());
}

TEST(BitflagShould, Reverse)
{
    bitflag first(6);
    first.set(3, true);

    bitflag second = !first;
    EXPECT_EQ("111011", second.str());
}

TEST(BitflagShould, CompareWithBitAndOperationLessThanByte)
{
    bitflag first(6);
    first.set(3, true);

    bitflag second(4);
    ASSERT_TRUE(first.has(second));

    second.set(0, true);
    ASSERT_FALSE(first.has(second));
    second.set(1, true);
    ASSERT_FALSE(first.has(second));
    second.set(2, true);
    ASSERT_FALSE(first.has(second));
    second.set(3, true);
    char val = first.has(second);
    ASSERT_FALSE(first.has(second));
    second.set(0, false);
    ASSERT_FALSE(first.has(second));
    second.set(1, false);
    ASSERT_FALSE(first.has(second));
    second.set(2, false);
    ASSERT_TRUE(first.has(second));
}

TEST(BitflagShould, CompareWithBitAndOperationTwoBytes)
{
    bitflag first(14);
    first.set(12, true);

    bitflag second(11);
    ASSERT_TRUE(first.has(second));

    first.set(10, true);
    ASSERT_TRUE(first.has(second));

    second.set(10, true);
    ASSERT_TRUE(first.has(second));

    first.set(1, true);
    ASSERT_FALSE(first.has(second));

    second.set(1, true);
    ASSERT_TRUE(first.has(second));
}
