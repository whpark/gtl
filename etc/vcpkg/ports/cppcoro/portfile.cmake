vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

if(VCPKG_TARGET_IS_LINUX)
    message("Warning: cppcoro requires libc++ and Clang on Linux. See https://github.com/microsoft/vcpkg/pull/10693#issuecomment-610394650.")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO            andreasbuhr/cppcoro
    REF             e86216e4fa6145f0184b5fef79230e9d4dc3aa77 # 2023-11-22
    SHA512          baa552d4984290811fc992cc14c9dfefbce81a277c66b01d965b9053ccde9d55a35d05ec938c9e1cb6fb3d59165c0b74be6839e97f4c6d9daf051259d8327385
    HEAD_REF        master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=False
)
vcpkg_cmake_install()

file(INSTALL     "${SOURCE_PATH}/LICENSE.txt"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME      copyright
)
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
