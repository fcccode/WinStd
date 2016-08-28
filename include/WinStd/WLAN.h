﻿/*
    Copyright 1991-2016 Amebis
    Copyright 2016 GÉANT

    This file is part of WinStd.

    Setup is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Setup is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Setup. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"

#include <wlanapi.h>

#include <string>

// Must not statically link to Wlanapi.dll as it is not available on Windows
// without a WLAN interface.
extern DWORD (WINAPI *pfnWlanReasonCodeToString)(__in DWORD dwReasonCode, __in DWORD dwBufferSize, __in_ecount(dwBufferSize) PWCHAR pStringBuffer, __reserved PVOID pReserved);

///
/// \defgroup WinStdWLANAPI WLAN API
/// Integrates WinStd classes with Microsoft WLAN API
///
/// @{

namespace winstd {
    ///
    /// Deleter for unique_ptr using WlanFreeMemory
    ///
    template <class _Ty> struct WlanFreeMemory_delete;

    ///
    /// Deleter for unique_ptr to array of unknown size using WlanFreeMemory
    ///
    template <class _Ty> struct WlanFreeMemory_delete<_Ty[]>;

    ///
    /// WLAN handle wrapper
    ///
    class WINSTD_API wlan_handle;
}

///
/// Retrieves a string that describes a specified reason code and stores it in a std::wstring string.
///
/// \sa [WlanReasonCodeToString function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms706768.aspx)
///
/// \note
/// Since Wlanapi.dll is not always present, the \c pfnWlanReasonCodeToString pointer to \c WlanReasonCodeToString
/// function must be loaded dynamically.
///
template<class _Elem, class _Traits, class _Ax> inline DWORD WlanReasonCodeToString(_In_ DWORD dwReasonCode, _Out_ std::basic_string<_Elem, _Traits, _Ax> &sValue, __reserved PVOID pReserved);

/// @}

#pragma once


namespace winstd
{
    template <class _Ty> struct WlanFreeMemory_delete
    {
        typedef WlanFreeMemory_delete<_Ty> _Myt; ///< This type

        ///
        /// Default construct
        ///
        WlanFreeMemory_delete() {}

        ///
        /// Construct from another WlanFreeMemory_delete
        ///
        template <class _Ty2> WlanFreeMemory_delete(const WlanFreeMemory_delete<_Ty2>&) {}

        ///
        /// Delete a pointer
        ///
        void operator()(_Ty *_Ptr) const
        {
            WlanFreeMemory(_Ptr);
        }
    };


    template <class _Ty> struct WlanFreeMemory_delete<_Ty[]>
    {
        typedef WlanFreeMemory_delete<_Ty> _Myt; ///< This type

        ///
        /// Default construct
        ///
        WlanFreeMemory_delete() {}

        ///
        /// Delete a pointer
        ///
        void operator()(_Ty *_Ptr) const
        {
            WlanFreeMemory(_Ptr);
        }

        ///
        /// Delete a pointer of another type
        ///
        template<class _Other>
        void operator()(_Other *) const
        {
            WlanFreeMemory(_Ptr);
        }
    };


    class WINSTD_API wlan_handle : public handle<HANDLE>
    {
    public:
        ///
        /// Initializes a new class instance with the object handle set to NULL.
        ///
        inline wlan_handle() : handle<HANDLE>() {}

        ///
        /// Initializes a new class instance with an already available object handle.
        ///
        /// \param[in] h  Initial object handle value
        ///
        inline wlan_handle(_In_opt_ handle_type h) : handle<HANDLE>(h) {}

        ///
        /// Move constructor
        ///
        /// \param[inout] h  A rvalue reference of another object
        ///
        inline wlan_handle(_Inout_ wlan_handle &&h) : handle<HANDLE>(std::move(h)) {}

        ///
        /// Closes a connection to the server.
        ///
        /// \sa [WlanCloseHandle function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms706610(v=vs.85).aspx)
        ///
        virtual ~wlan_handle();

        ///
        /// Move assignment
        ///
        /// \param[inout] h  A rvalue reference of another object
        ///
        wlan_handle& operator=(_Inout_ wlan_handle &&h)
        {
            if (this != std::addressof(h))
                *(handle<handle_type>*)this = std::move(h);
            return *this;
        }

        ///
        /// Opens a connection to the server.
        ///
        /// \sa [WlanOpenHandle function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms706759.aspx)
        ///
        /// \return
        /// - \c true when succeeds;
        /// - \c false when fails. Use `GetLastError()` for failure reason.
        ///
        inline bool open(_In_ DWORD dwClientVersion, _Out_ PDWORD pdwNegotiatedVersion)
        {
            handle_type h;
            DWORD dwResult = WlanOpenHandle(dwClientVersion, 0, pdwNegotiatedVersion, &h);
            if (dwResult == ERROR_SUCCESS) {
                attach(h);
                return true;
            } else {
                SetLastError(dwResult);
                return false;
            }
        }


    private:
        // This class is noncopyable.
        wlan_handle(_In_ const wlan_handle &h);
        wlan_handle& operator=(_In_ const wlan_handle &h);

    protected:
        ///
        /// Closes a connection to the server.
        ///
        /// \sa [WlanCloseHandle function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms706610(v=vs.85).aspx)
        ///
        virtual void free_internal();
    };
}


template<class _Elem, class _Traits, class _Ax>
inline DWORD WlanReasonCodeToString(_In_ DWORD dwReasonCode, _Out_ std::basic_string<_Elem, _Traits, _Ax> &sValue, __reserved PVOID pReserved)
{
    assert(0); // TODO: Test this code.

    DWORD dwSize = 0;

    if (!::pfnWlanReasonCodeToString)
        return ERROR_CALL_NOT_IMPLEMENTED;

    for (;;) {
        // Increment size and allocate buffer.
        auto szBuffer = std::unique_ptr<_Elem[]>(new _Elem[dwSize += 1024]);

        // Try!
        DWORD dwResult = ::pfnWlanReasonCodeToString(dwReasonCode, dwSize, szBuffer.get(), pReserved);
        if (dwResult == ERROR_SUCCESS) {
            DWORD dwLength = (DWORD)wcsnlen(szBuffer.get(), dwSize);
            if (dwLength < dwSize) {
                // Buffer was long enough.
                sValue.assign(szBuffer.get(), dwLength);
                return ERROR_SUCCESS;
            }
        } else {
            // Return error code.
            return dwResult;
        }
    }
}
