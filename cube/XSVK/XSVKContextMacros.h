#pragma once

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#ifdef _WIN32
#define ERR_EXIT(err_msg, err_class)											\
	do																			\
	{																			\
		MessageBox(nullptr, err_msg, err_class, MB_OK);							\
		exit(1);																\
	}																			\
	while (0)

#else // _WIN32

#define ERR_EXIT(err_msg, err_class)											\
	do																			\
	{																			\
		printf(err_msg);														\
		fflush(stdout);															\
		exit(1);																\
	}																			\
	while (0)
#endif // _WIN32

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)								\
	{																			\
		m_Fx##entrypoint =														\
			(PFN_vk##entrypoint)vkGetInstanceProcAddr(							\
				m_pVkInst, "vk" #entrypoint);									\
		if (m_Fx##entrypoint == nullptr)										\
		{																		\
			ERR_EXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint,		\
					 "vkGetInstanceProcAddr Failure");							\
		}																		\
	}

static PFN_vkGetDeviceProcAddr g_gdpa = nullptr;

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)									\
	{																			\
		if (!g_gdpa)															\
			g_gdpa = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(			\
				m_pVkInst, "vkGetDeviceProcAddr");								\
		m_Fx##entrypoint =														\
			(PFN_vk##entrypoint)g_gdpa(dev, "vk" #entrypoint);					\
		if (m_Fx##entrypoint == nullptr)										\
		{																		\
			ERR_EXIT("vkGetDeviceProcAddr failed to find vk" #entrypoint,		\
					 "vkGetDeviceProcAddr Failure");							\
		}																		\
	}
