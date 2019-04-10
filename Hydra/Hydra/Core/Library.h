#pragma once

#pragma warning(disable:4251) //TODO: Zeptat se Adama na tenhle error!

#pragma warning(disable:4267) // Disable possible lost of data
#pragma warning(disable:4244) // Disable possible lost of data
#pragma warning(disable:4102) // Disable unreferenced label

#ifdef HYDRA_EXPORTS
#define HYDRA_EXPORT __declspec(dllexport)
#else
#define HYDRA_EXPORT __declspec(dllimport)
#endif