#include "MFile.h"
#include "TestAssert.h"

#ifdef CreateFile
#undef CreateFile
#endif

namespace TestMFileInternal {
namespace {

#include "TestRandom.h"

void Test1(ArrayView<char> FilenameBuf, bool File)
{
	auto Filename = FilenameBuf.data();
	char NewFilename[MFile::MaxPath];
	sprintf_safe(NewFilename, "%s_moved", Filename);
	for (auto Name : {Filename, NewFilename})
		MFile::Delete(Name);
	TestAssert((File ? MFile::CreateFile : MFile::CreateDir)(Filename));
	TestAssert(MFile::IsFile(Filename) == File);
	TestAssert(MFile::IsDir(Filename) == !File);
	TestAssert(MFile::File{}.open(Filename) == File);
	TestAssert(MFile::Move(Filename, NewFilename));
	TestAssert(!MFile::IsFile(Filename) && !MFile::IsDir(Filename));
	TestAssert(!MFile::File{}.open(Filename));
	TestAssert(!MFile::Delete(Filename));
	strcpy_safe(FilenameBuf, NewFilename);
}

constexpr auto WriteSize = 32;

void TestWrite(const char* Filename, bool Text)
{
	char StrBuf[WriteSize];
	ArrayView<char> Str = StrBuf;
	RandomString(Str);
	Str.back() = '\n';
	{
		auto Flags = MFile::Clear;
		if (Text)
			Flags |= MFile::Text;
		MFile::RWFile File;
		TestAssert(File.open(Filename, Flags));
		File.write(Str.data(), Str.size());
		TestAssert(!File.error());
	}
	{
		MFile::File File;
		if (Text)
			TestAssert(File.open(Filename, MFile::Text));
		else
			TestAssert(File.open(Filename));
		char Str2Buf[WriteSize * 2];
		ArrayView<char> Str2 = Str2Buf;
		auto ReadSz = File.read(Str2.data(), Str2.size());
		Str2 = Str2.subview(0, ReadSz);
		TestAssert(Str == Str2);
	}
}

constexpr auto NumFiles = 10;

void CheckFiles(char (&Filenames)[NumFiles][64], size_t Size)
{
	bool FoundFilenames[NumFiles]{};

	for (auto& File : MFile::Glob("*"))
	{
		TestAssert(!equals(File.Name, ".") && !equals(File.Name, ".."));
        int i = 0;
		for (auto& Filename : Filenames)
		{
			if (iequals(File.Name, Filename))
			{
				FoundFilenames[i] = true;
				TestAssert(File.Size == Size);
				break;
			}
            ++i;
		}
	}

	for (bool b : FoundFilenames)
		TestAssert(b);
}

} // namespace
} // namespace TestMFileInternal

void TestMFile()
{
	using namespace TestMFileInternal;
	char Filenames[NumFiles][64];
	for (bool File : {false, true})
	{
		for (auto& Filename : Filenames)
		{
			auto View = ArrayView<char>(Filename);
			View.remove_suffix(10);
			RandomString(View);
			Test1(Filename, File);
		}
	}

	CheckFiles(Filenames, 0);

	for (bool Text : {false, true})
	{
		for (auto&& Filename : Filenames)
		{
			TestWrite(Filename, Text);
		}

#ifdef _WIN32
		int Offset = int(Text);
#else
		int Offset = 0;
#endif
		CheckFiles(Filenames, WriteSize + Offset);
	}

	for (auto&& Filename : Filenames)
		TestAssert(MFile::Delete(Filename));
}
