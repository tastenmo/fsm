# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import sys
import sphinx
import datetime

sys.path.append(os.path.dirname(__file__))  # Needed for test_py_module

# project = 'Sphinx-SimplePDF-DEMO'
project = 'fsm'
copyright = '2023, ESCAD Automation GmbH'
author = 'Martin Heubuch'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_escaddocs',
    'sphinxcontrib.plantuml',
    'sphinx_needs',
    'sphinx.ext.autodoc',
    'sphinx.ext.imgmath',
    'breathe'
]

version = "1.0"  # Will not be raised

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

breathe_projects = {"fsm": "./doc_out/xml/"}
breathe_default_project = "fsm"

html_context = {   
    'docs_scope': 'external',
    'cover_logo_title': '',
    'cover_meta_data': 'The ESCAD fsm library',
    'cover_footer': f'Build: {datetime.datetime.now().strftime("%d.%m.%Y")}<br>'
                    f'Maintained by <a href="https://www.escad.de">ESCAD Automation GmbH</a>',                                                              
    'authors' : [
        ("Martin Heubuch", "Softwareentwicklung", "martin.heubuch@escad.de"),
        ("Thomas Strohmaier", "Softwareentwicklung", "thomas.strohmaier@escad.de"),
        ("", "", "")
    ], 
    'reviewer' : [
        ("Manfred Hirt", "Bereichsleiter Steuerungstechnik ", "manfred.hirt@escad.de"),
        ("Peter Herr", "Stv. Abteilungsleiter Software", "peter.herr@escad.de"),
        ("", "", "")
    ],
    'customer_approver'  : [
        ("", "", ""),
        ("", "", ""),
        ("", "", "")
    ]                                
}  

plantuml_output_format = "svg_img"

simplepdf_theme = 'escaddocs_theme'

simplepdf_debug = True

simplepdf_file_name = 'ESCAD_fsm_library.pdf'

simplepdf_vars = {
    'cover-overlay': 'rgba(84, 84, 84, 0.7)',
    'primary-opaque': 'rgba(211, 77, 40, 0.7)',
    'cover-logo' : '',
    'cover-bg': 'url(BackgroundReportA4.png) no-repeat left',
    'primary': '#d34c27',
    'secondary': '#54bfd3',
    'cover': '#ffffff',
    'white': '#ffffff',
    'links': '#54bfd3',
    'top-left-content': 'url(REscad.svg) no-repeat left center',
    'top-center-content': '',
    'top-right-content': '',
    'bottom-left-content': '"ESCAD_fsm_library.pdf"',
    'bottom-center-content': '',
    'bottom-right-content': 'counter(page)',
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'pydata_sphinx_theme'
html_static_path = ['_static']

html_logo = './_static/REscad.svg'

plantuml_output_format = "svg_img"
local_plantuml_path = os.path.join(os.path.dirname(__file__), "../", "docs", "utils", "plantuml.jar")
plantuml = f"java -Djava.awt.headless=true -jar {local_plantuml_path}"

imgmath_image_format = 'svg'

def setup_jquery(app, exception):
    """
    Inject jQuery if Sphinx>=6.x

    Staring on Sphinx 6.0, jQuery is not included with it anymore.
    As this extension depends on jQuery, we are including it when Sphinx>=6.x
    """

    if sphinx.version_info >= (5, 0, 0):
        # https://jquery.com/download/#using-jquery-with-a-cdn
        jquery_cdn_url = "https://code.jquery.com/jquery-3.6.0.min.js"
        html_js_files = getattr(app.config, "html_js_files", [])
        html_js_files.append((
            jquery_cdn_url,
            {
                'integrity': 'sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4=',
                'crossorigin': 'anonymous'
            }
        ))
        app.config.html_js_files = html_js_files

