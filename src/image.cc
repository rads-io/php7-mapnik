#include "exception.h"
#include "image.h"
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>

zend_class_entry *image_ce;
zend_object_handlers image_object_handlers;

// Class methods

PHP_METHOD(Image, __construct)
{
    image_object *obj = Z_PHP_MAPNIK_IMAGE_P(getThis());
    zend_long width, height;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    obj->image = new mapnik::image_rgba8(width, height);
}

PHP_METHOD(Image, saveToFile)
{
    image_object *obj = Z_PHP_MAPNIK_IMAGE_P(getThis());
    zend_string *file;
    zend_string *format = zend_string_init("png", sizeof("png") - 1, 1);

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
        Z_PARAM_STR(file)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(format)
    ZEND_PARSE_PARAMETERS_END();

    std::string file_str(ZSTR_VAL(file), ZSTR_LEN(file));
    std::string format_str(ZSTR_VAL(format), ZSTR_LEN(format));

    try {
        mapnik::save_to_file(*obj->image, file_str, format_str);
    } catch (const mapnik::image_writer_exception & ex) {
        throw_mapnik_exception(ex.what());
        return;
    } catch (const std::exception & ex) {
        throw_mapnik_exception(ex.what());
        return;
    } catch (...) {
        throw_mapnik_exception("Unknown exception thrown while saving image to file.");
        return;
    }

    RETURN_TRUE;
}

PHP_METHOD(Image, saveToString)
{
    image_object *obj = Z_PHP_MAPNIK_IMAGE_P(getThis());
    zend_string *format = zend_string_init("png", sizeof("png") - 1, 1);
    std::string image_str;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(format)
    ZEND_PARSE_PARAMETERS_END();

    try {
        std::string format_str(ZSTR_VAL(format), ZSTR_LEN(format));
        image_str = mapnik::save_to_string(*obj->image, format_str);
    } catch (const mapnik::image_writer_exception & ex) {
        throw_mapnik_exception(ex.what());
        return;
    } catch (const std::exception & ex) {
        throw_mapnik_exception(ex.what());
        return;
    } catch (...) {
        throw_mapnik_exception("Unknown exception thrown while saving image to string.");
        return;
    }

    RETURN_STRINGL(image_str.c_str(), image_str.size());
}

// Reflection info

ZEND_BEGIN_ARG_INFO_EX(argInfo_image_construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
    ZEND_ARG_INFO(0, width)
    ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_image_saveToFile, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_INFO(0, file)
    ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_image_saveToString, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

// Register methods

zend_function_entry image_methods[] = {
    PHP_ME(Image, __construct, argInfo_image_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(Image, saveToFile, argInfo_image_saveToFile, ZEND_ACC_PUBLIC)
    PHP_ME(Image, saveToString, argInfo_image_saveToString, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

// Internal object handling

void free_image(zend_object *object TSRMLS_DC)
{
    image_object *obj;
    obj = fetch_image_object(object);
    delete obj->image;
    zend_object_std_dtor(object TSRMLS_DC);
}

zend_object * create_image(zend_class_entry *ce TSRMLS_DC) {
    image_object *intern;
    intern = (image_object*) ecalloc(1, sizeof(image_object) + zend_object_properties_size(ce));

    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &image_object_handlers;

    return &intern->std;
}

// Extension class initializer

void init_image(INIT_FUNC_ARGS)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Mapnik", "Image", image_methods);
    image_ce = zend_register_internal_class(&ce TSRMLS_CC);
    image_ce->create_object = create_image;

    memcpy(&image_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    image_object_handlers.offset = XtOffsetOf(struct image_object, std);
    image_object_handlers.free_obj = &free_image;
    image_object_handlers.clone_obj = NULL;
}
