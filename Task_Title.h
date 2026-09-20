#pragma warning(disable:4996)
#pragma once

#include "GameEngine_Ver3_83.h"

namespace Title
{
	const string defGroupName("Scene");
	const string defName("Title");

	class Resource : public BResource
	{
		bool Initialize() override;
		bool Finalize() override;
		Resource();
	public:
		~Resource();
		using SP = shared_ptr<Resource>;
		using WP = weak_ptr<Resource>;
		static WP instance;
		static SP Create();

		DG::Image::SP img;
	};

	class Object : public BTask
	{
	public:
		~Object() override;
		using SP = shared_ptr<Object>;
		using WP = weak_ptr<Object>;
		static SP Create(bool flagGameEnginePushBack_);

		Resource::SP res;
	private:
		Object();
		bool B_Initialize();
		bool B_Finalize();
		bool Initialize();
		void UpDate() override;
		void Render2D_AF() override;
		bool Finalize();

		int logoPosY;
	};
}
