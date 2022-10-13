{
    "targets": [
        {
            "target_name": "macrootcas_native",
            "sources": ["src/binding.cc"],
            "xcode_settings": {
                "OTHER_CFLAGS": [
                    "-arch x86_64",
                    "-arch arm64",
                    "-mmacosx-version-min=10.14"
                ],
                "OTHER_LDFLAGS": [
                    "-framework CoreFoundation",
                    "-framework Security",
                    "-arch x86_64",
                    "-arch arm64",
                    "-mmacosx-version-min=10.14"
                ]
            }
        }
    ]
}