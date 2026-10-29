p3d_add_test_executable(test_calibration_store
    SOURCES test_calibration_store.cpp
    DEPS p3d::domain::calibration_store p3d::hal_test_support
)
