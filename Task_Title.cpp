#include "MyPG.h"
#include "Task_Title.h"
#include "Task_Game.h"

namespace Title
{
	Resource::WP Resource::instance;

	bool Resource::Initialize()
	{
		img = DG::Image::Create("./data/image/Medamayaki_Title.png");
		return img != nullptr;
	}

	bool Resource::Finalize()
	{
		img.reset();
		return true;
	}

	bool Object::Initialize()
	{
		__super::Initialize(defGroupName, defName, true);
		res = Resource::Create();
		logoPosY = -1080;
		return res != nullptr && res->img != nullptr;
	}

	bool Object::Finalize()
	{
		res.reset();
		if (!ge->QuitFlag() && nextTaskCreate) {
			Game::Object::Create(true);
		}
		return true;
	}

	void Object::UpDate()
	{
		logoPosY += 27;
		if (logoPosY > 0) {
			logoPosY = 0;
		}

		const auto input = ge->in1->GetState();
		if (input.B1.down) {
			Kill();
		}
	}

	void Object::Render2D_AF()
	{
		ML::Box2D draw(0, 0, 1920, 1080);
		draw.Offset(0, logoPosY);
		const POINT imageSize = res->img->Size();
		const ML::Box2D src(0, 0, imageSize.x, imageSize.y);
		res->img->Draw(draw, src);
	}

	Object::SP Object::Create(bool flagGameEnginePushBack_)
	{
		Object::SP object(new Object());
		if (object) {
			object->me = object;
			if (flagGameEnginePushBack_) {
				ge->PushBack(object);
			}
			if (!object->B_Initialize()) {
				object->Kill(false);
			}
			return object;
		}
		return nullptr;
	}

	bool Object::B_Initialize()
	{
		return Initialize();
	}

	Object::~Object()
	{
		B_Finalize();
	}

	bool Object::B_Finalize()
	{
		return Finalize();
	}

	Object::Object()
	{
	}

	Resource::SP Resource::Create()
	{
		if (auto resource = instance.lock()) {
			return resource;
		}

		Resource::SP resource(new Resource());
		if (resource && resource->Initialize()) {
			instance = resource;
			return resource;
		}
		return nullptr;
	}

	Resource::Resource()
	{
	}

	Resource::~Resource()
	{
		Finalize();
	}
}
