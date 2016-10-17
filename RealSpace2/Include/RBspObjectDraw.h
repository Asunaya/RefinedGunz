#pragma once

#include "VariantPolymorphism.h"
#include "RBspObjectDrawD3D9.h"
#include "RBspObjectDrawVulkan.h"
#include "RNameSpace.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;

class RBspObjectDraw
{
public:
	RBspObjectDraw(D3D9Tag, RBspObject& bsp) : var{ RBspObjectDrawD3D9{bsp} } {}
	RBspObjectDraw(VulkanTag, RBspObject& bsp) : var{ RBspObjectDrawVulkan{bsp} } {}

	template <typename T>
	auto& Get() { return var.get_ref<T>(); }

	POLYVAR_METHOD(Init)
	POLYVAR_METHOD(Draw)

private:
	variant<RBspObjectDrawD3D9, RBspObjectDrawVulkan> var;
};

_NAMESPACE_REALSPACE2_END