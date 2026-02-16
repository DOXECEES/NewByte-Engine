#include "String.hpp"
#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <benchmark/benchmark.h>



// -----------------------------
// 1. Пустая строка
// -----------------------------
TEST(BasicStringTest, EmptyString)
{
    BasicString<char> s;
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.empty());
    EXPECT_STREQ(s.c_str(), "");
}

// -----------------------------
// 2. SSO (короткая строка)
// -----------------------------
TEST(BasicStringTest, SSOString)
{
    BasicString<char> s("Hello");
    EXPECT_EQ(s.size(), 5);
    EXPECT_FALSE(s.empty());
    EXPECT_STREQ(s.c_str(), "Hello");

    s[0] = 'h';
    EXPECT_STREQ(s.c_str(), "hello");

    s.clear();
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.empty());
    EXPECT_STREQ(s.c_str(), "");
}

// -----------------------------
// 3. Heap (длинная строка)
// -----------------------------
TEST(BasicStringTest, HeapString)
{
    std::string longStr(100, 'x');
    BasicString<char> s(longStr.c_str());

    EXPECT_EQ(s.size(), 100);
    EXPECT_FALSE(s.empty());
    EXPECT_STREQ(s.c_str(), longStr.c_str());

    s[0] = 'y';
    EXPECT_EQ(s[0], 'y');
}

// -----------------------------
// 4. Конструктор копирования
// -----------------------------
TEST(BasicStringTest, CopyConstructor)
{
    BasicString<char> s1("CopyMe");
    BasicString<char> s2(s1);

    EXPECT_EQ(s2.size(), s1.size());
    EXPECT_STREQ(s2.c_str(), s1.c_str());

    s2[0] = 'c';
    EXPECT_STRNE(s2.c_str(), s1.c_str());
}

// -----------------------------
// 5. Конструктор перемещения
// -----------------------------
TEST(BasicStringTest, MoveConstructor)
{
    BasicString<char> s1("MoveMe");
    BasicString<char> s2(std::move(s1));

    EXPECT_STREQ(s2.c_str(), "MoveMe");
    EXPECT_EQ(s1.size(), 0);  // перемещённый объект пустой
    EXPECT_TRUE(s1.empty());
    EXPECT_STREQ(s1.c_str(), "");
}

// -----------------------------
// 6. Оператор присваивания (копирование)
// -----------------------------
TEST(BasicStringTest, CopyAssignment)
{
    BasicString<char> s1("AssignMe");
    BasicString<char> s2;
    s2 = s1;

    EXPECT_STREQ(s2.c_str(), "AssignMe");

    s2[0] = 'a';
    EXPECT_STRNE(s2.c_str(), s1.c_str());
}

// -----------------------------
// 7. Оператор присваивания (перемещение)
// -----------------------------
TEST(BasicStringTest, MoveAssignment)
{
    BasicString<char> s1("TempHeapStringThatExceedsSSO");
    BasicString<char> s2;
    s2 = std::move(s1);

    EXPECT_STREQ(s2.c_str(), "TempHeapStringThatExceedsSSO");
    EXPECT_EQ(s1.size(), 0);
    EXPECT_TRUE(s1.empty());
    EXPECT_STREQ(s1.c_str(), "");
}

// -----------------------------
// 8. Очень длинная строка (Heap)
// -----------------------------
TEST(BasicStringTest, VeryLongString)
{
    std::string longStr(10000, 'z');
    BasicString<char> s(longStr.c_str());

    EXPECT_EQ(s.size(), 10000);
    EXPECT_STREQ(s.c_str(), longStr.c_str());

    s[9999] = 'y';
    EXPECT_EQ(s[9999], 'y');
}

// -----------------------------
// 9. UTF-16 строка (SSO)
// -----------------------------
TEST(BasicStringTest, UTF16SSO)
{
    const char16_t text[] = u"Hello16";
    BasicString<char16_t> s(text);

    EXPECT_EQ(s.size(), 7);
    EXPECT_EQ(s[0], u'H');

    s[0] = u'h';
    EXPECT_EQ(s[0], u'h');
}

// -----------------------------
// 10. UTF-32 строка (SSO)
// -----------------------------
TEST(BasicStringTest, UTF32SSO)
{
    const char32_t text[] = U"Hello32";
    BasicString<char32_t> s(text);

    EXPECT_EQ(s.size(), 7);
    EXPECT_EQ(s[0], U'H');

    s[0] = U'h';
    EXPECT_EQ(s[0], U'h');
}

// -----------------------------
// 11. Смешанное: SSO ↔ Heap переход
// -----------------------------
TEST(BasicStringTest, SSOtoHeapTransition)
{
    BasicString<char> s("short"); // SSO
    std::string longStr(100, 'x');

    s = BasicString<char>(longStr.c_str()); // перемещаемся на Heap
    EXPECT_EQ(s.size(), 100);
    EXPECT_STREQ(s.c_str(), longStr.c_str());
}

// -----------------------------------
// PUSH_BACK тесты
// -----------------------------------

TEST(BasicStringTest, PushBackSingle)
{
    BasicString<char> s;
    s.pushBack('A');

    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s[0], 'A');
    EXPECT_STREQ(s.c_str(), "A");
}

TEST(BasicStringTest, PushBackMultiple)
{
    BasicString<char> s;

    for (char c = 'a'; c <= 'z'; ++c)
        s.pushBack(c);

    EXPECT_EQ(s.size(), 26);
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s[25], 'z');
    EXPECT_EQ(std::strncmp(s.c_str(), "abcdefghijklmnopqrstuvwxyz", 26), 0);
}

TEST(BasicStringTest, PushBackMaintainsNullTerminator)
{
    BasicString<char> s("abc");
    s.pushBack('d');

    EXPECT_EQ(s.size(), 4);
    EXPECT_EQ(s[3], 'd');
    EXPECT_EQ(s.c_str()[4], '\0');
}

TEST(BasicStringTest, PushBackTriggersSSOToHeapTransition)
{
    // создаём короткую строку (SSO)
    BasicString<char> s("short");

    // добавляем символы до перехода в heap
    for (int i = 0; i < 64; ++i)
        s.pushBack('x');

    EXPECT_GE(s.size(), 64);
    EXPECT_EQ(s.c_str()[s.size()], '\0');
}

TEST(BasicStringTest, PushBackAfterHeapAllocation)
{
    std::string base(100, 'a');
    BasicString<char> s(base.c_str());

    size_t prevCap = s.capacity();
    s.pushBack('b');

    EXPECT_EQ(s[s.size() - 1], 'b');
    EXPECT_EQ(s.c_str()[s.size()], '\0');
    EXPECT_GE(s.capacity(), prevCap);
}

TEST(BasicStringTest, PushBackAfterMove)
{
    BasicString<char> s("move");
    BasicString<char> moved(std::move(s));

    // moved — валиден, s должен быть пуст
    moved.pushBack('!');

    EXPECT_STREQ(moved.c_str(), "move!");
    EXPECT_TRUE(s.empty());
    EXPECT_STREQ(s.c_str(), "");
}

TEST(BasicStringTest, PushBackEmptyToHeap)
{
    BasicString<char> s;
    for (int i = 0; i < 128; ++i)
        s.pushBack('Z');

    EXPECT_EQ(s.size(), 128);
    EXPECT_EQ(s[127], 'Z');
    EXPECT_EQ(s.c_str()[128], '\0');
}

TEST(BasicStringTest, PushBackStressTest)
{
    BasicString<char> s;
    constexpr int N = 10000;

    for (int i = 0; i < N; ++i)
        s.pushBack('x');

    EXPECT_EQ(s.size(), N);
    EXPECT_EQ(s[0], 'x');
    EXPECT_EQ(s[N - 1], 'x');
    EXPECT_EQ(s.c_str()[N], '\0');
}

TEST(BasicStringTest, PushBackBenchmark)
{


}

// -------------------------------------------------------
// insert(size_t pos, const T& ch)
// -------------------------------------------------------
TEST(BasicStringInsertTest, InsertSingleCharSSO)
{
    BasicString<char> s("Hell");
    s.insert(2, 'X');
    EXPECT_STREQ(s.c_str(), "HeXll");
    EXPECT_EQ(s.size(), 5);
}

TEST(BasicStringInsertTest, InsertSingleCharHeap)
{
    std::string base(100, 'a');
    BasicString<char> s(base.c_str());
    s.insert(50, 'X');

    EXPECT_EQ(s.size(), 101);
    EXPECT_EQ(s[50], 'X');
    EXPECT_EQ(s[49], 'a');
    EXPECT_EQ(s[51], 'a');
}

// -------------------------------------------------------
// insert(size_t pos, const T* str)
// -------------------------------------------------------
TEST(BasicStringInsertTest, InsertCStringIntoSSO)
{
    BasicString<char> s("Hello");
    s.insert(5, "World");

    EXPECT_STREQ(s.c_str(), "HelloWorld");
    EXPECT_EQ(s.size(), 10);
}

TEST(BasicStringInsertTest, InsertCStringIntoHeap)
{
    std::string base(200, 'x');
    BasicString<char> s(base.c_str());
    s.insert(100, "INSERT");

    EXPECT_EQ(s.size(), 206);
    EXPECT_TRUE(std::string(s.c_str()).find("INSERT") != std::string::npos);
}

// -------------------------------------------------------
// insert(size_t pos, const T* str, size_t count)
// -------------------------------------------------------
TEST(BasicStringInsertTest, InsertCStringWithCountSSO)
{
    BasicString<char> s("Hello");
    s.insert(2, "ABCDE", 3); // вставим "ABC", начиная с 2

    EXPECT_STREQ(s.c_str(), "HeABCllo");
    EXPECT_EQ(s.size(), 8);
}

TEST(BasicStringInsertTest, InsertCStringWithCountHeap)
{
    std::string base(80, 'q');
    BasicString<char> s(base.c_str());
    s.insert(40, "TEST", 2); // вставим "TE"

    EXPECT_EQ(s.size(), 82);
    EXPECT_EQ(s[40], 'T');
    EXPECT_EQ(s[41], 'E');
}

// -------------------------------------------------------
// insert(size_t pos, const BasicString& other)
// -------------------------------------------------------
TEST(BasicStringInsertTest, InsertOtherStringSSO)
{
    BasicString<char> s1("Good");
    BasicString<char> s2("Morning");

    s1.insert(4, s2);

    EXPECT_STREQ(s1.c_str(), "GoodMorning");
    EXPECT_EQ(s1.size(), 11);
}

TEST(BasicStringInsertTest, InsertOtherStringHeap)
{
    std::string base(120, 'a');
    std::string insertStr(30, 'Z');

    BasicString<char> s1(base.c_str());
    BasicString<char> s2(insertStr.c_str());

    s1.insert(60, s2);

    EXPECT_EQ(s1.size(), 150);
    for (size_t i = 60; i < 90; ++i)
    {
        EXPECT_EQ(s1[i], 'Z');
    }
}

// -------------------------------------------------------
// Граничные случаи
// -------------------------------------------------------
TEST(BasicStringInsertTest, InsertAtBeginning)
{
    BasicString<char> s("World");
    s.insert(0, "Hello");
    EXPECT_STREQ(s.c_str(), "HelloWorld");
}

TEST(BasicStringInsertTest, InsertAtEnd)
{
    BasicString<char> s("Hello");
    s.insert(s.size(), "World");
    EXPECT_STREQ(s.c_str(), "HelloWorld");
}

TEST(BasicStringInsertTest, InsertEmptyString)
{
    BasicString<char> s("Hello");
    s.insert(3, ""); // вставляем пустую
    EXPECT_STREQ(s.c_str(), "Hello");
    EXPECT_EQ(s.size(), 5);
}

TEST(BasicStringInsertTest, InsertEmptyOther)
{
    BasicString<char> s1("Data");
    BasicString<char> s2("");
    s1.insert(2, s2);
    EXPECT_STREQ(s1.c_str(), "Data");
}

TEST(BasicStringInsertTest, InsertOutOfBoundsShouldAppend)
{
    BasicString<char> s("Test");
    EXPECT_DEATH({
        s.insert(999, "End");
        }, "");
}

TEST(BasicStringInsertTest, InsertCharOutOfBoundsShouldAssert)
{
    BasicString<char> s("A");

    EXPECT_DEATH({
        s.insert(50, 'B');
        }, ""); 
}

TEST(BasicStringEraseTest, EraseSingleChar)
{
    BasicString<char> s("Hello");
    s.erase(1); // удаляем 'e'
    EXPECT_STREQ(s.c_str(), "Hllo");
}

TEST(BasicStringEraseTest, EraseRangeWithinBounds)
{
    BasicString<char> s("HelloWorld");
    s.erase(5, 5); // удаляем "World"
    EXPECT_STREQ(s.c_str(), "Hello");
}

TEST(BasicStringEraseTest, EraseRangeExceedingBounds)
{
    BasicString<char> s("ABCDE");
    s.erase(2, 10); // пытаемся удалить больше, чем осталось
    EXPECT_STREQ(s.c_str(), "AB");
}

TEST(BasicStringEraseTest, EraseOutOfBounds)
{
    BasicString<char> s("A");
    EXPECT_DEATH(s.erase(10), "erase\\(\\) position out of range"); // NB_ASSERT
}


TEST(BasicStringReplaceTest, ReplaceExistingSubstring)
{
    BasicString<char> s("Hello World");
    BasicString<char> pattern("World");
    BasicString<char> replacement("Engine");

    s.replace(pattern, replacement);

    EXPECT_STREQ(s.c_str(), "Hello Engine");
}

TEST(BasicStringReplaceTest, ReplaceAtStart)
{
    BasicString<char> s("ABCDEF");
    BasicString<char> pattern("ABC");
    BasicString<char> replacement("123");

    s.replace(pattern, replacement);

    EXPECT_STREQ(s.c_str(), "123DEF");
}

TEST(BasicStringReplaceTest, ReplaceAtEnd)
{
    BasicString<char> s("NewByte");
    BasicString<char> pattern("Byte");
    BasicString<char> replacement("Engine");

    s.replace(pattern, replacement);

    EXPECT_STREQ(s.c_str(), "NewEngine");
}

TEST(BasicStringReplaceTest, ReplaceNonExisting)
{
    BasicString<char> s("NoMatchHere");
    BasicString<char> pattern("Missing");
    BasicString<char> replacement("Found");

    s.replace(pattern, replacement);

    EXPECT_STREQ(s.c_str(), "NoMatchHere");
}

TEST(BasicStringReplaceTest, ReplaceWholeString)
{
    BasicString<char> s("AAA");
    BasicString<char> pattern("AAA");
    BasicString<char> replacement("BBB");

    s.replace(pattern, replacement);

    EXPECT_STREQ(s.c_str(), "BBB");
}

TEST(BasicStringReplaceTest, ReplaceEmptyPatternShouldDoNothing)
{
    BasicString<char> s("Data");
    BasicString<char> pattern("");
    BasicString<char> replacement("X");

    s.replace(pattern, replacement);

    EXPECT_STREQ(s.c_str(), "Data");
}
TEST(BasicStringReplaceTest, ReplaceWithEmptyReplacement)
{
    



    ///BasicString<char> s("EraseMe");
    //BasicString<char> pattern("Me");
   // BasicString<char> replacement("");

    //s.replace(pattern, replacement);

    //EXPECT_STREQ(s.c_str(), "Erase");
}


static std::string make_large_text(size_t size)
{
    std::string s;
    s.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        s.push_back((i % 20 == 0) ? 'a' : 'b');
    }
    return s;
}

static void BM_StdStringReplace(benchmark::State& state)
{
    const std::string pattern = "ababa";
    const std::string replacement = "XYZ";

    // создаём большую строку
    std::string largeText = make_large_text(state.range(0));

    for (auto _ : state)
    {
        std::string s = largeText;
        size_t pos = s.find(pattern);
        if (pos != std::string::npos)
        {
            s.replace(pos, pattern.size(), replacement);
        }
        //benchmark::DoNotOptimize(s);
    }

    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_StdStringReplace)->Range(1 << 10, 1 << 20); // от 1K до 1M символов


static void BM_BasicStringReplace(benchmark::State& state)
{
    const BasicString<char> pattern = "ababa";
    const BasicString<char> replacement = "XYZ";

    BasicString<char> largeText;
    largeText.reserve(state.range(0));
    for (size_t i = 0; i < (size_t)state.range(0); ++i)
    {
        largeText.pushBack((i % 20 == 0) ? 'a' : 'b');
    }

    for (auto _ : state)
    {
        BasicString s = largeText;
        s.replace(pattern, replacement);
        //benchmark::DoNotOptimize(s);
    }

    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BasicStringReplace)->Range(1 << 10, 1 << 20);

static void BM_Replace_Find(benchmark::State& state)
{
    BasicString<char> text("hello world");
    BasicString<char> pattern("world");
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(kmpFind(text.c_str(), text.size(), pattern.c_str(), pattern.size()));
    }
}
BENCHMARK(BM_Replace_Find);

static void BM_Replace_Erase(benchmark::State& state)
{
    BasicString<char> text("hello world");
    for (auto _ : state)
    {
        auto copy = text;
        copy.erase(5, 5);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Replace_Erase);
static std::string generateText(size_t size)
{
    std::string s(size, 'a');
    s[size / 2] = 'b';
    return s;
}

// 🔹 Тест стандартного std::string::find
static void BM_StdStringFind(benchmark::State& state)
{
    auto text = generateText(state.range(0));
    std::string pattern = "b";

    for (auto _ : state)
    {
        text.find(pattern);
    }
}
BENCHMARK(BM_StdStringFind)->Arg(1000)->Arg(10000)->Arg(100000);

// 🔹 Тест твоего KMP
static void BM_BMHFind(benchmark::State& state)
{
    BasicString<char> text(generateText(state.range(0)).c_str());
    BasicString<char> pattern("b");

    for (auto _ : state)
    {
        bmhFindFast(text.c_str(), text.size(), pattern.c_str(), pattern.size());
    }
}
BENCHMARK(BM_BMHFind)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();

