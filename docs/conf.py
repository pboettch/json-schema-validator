project = 'Json Schema Validator'
copyright = '2023, Patrick Boettcher'
author = 'Patrick Boettcher'

extensions = [
    "myst_parser",
]

templates_path = []
exclude_patterns = [
    'build',
    '_build',
    'Thumbs.db',
    '.DS_Store',
    "README.md",
]
source_suffix = [".md"]


html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

myst_enable_extensions = [
    "tasklist",
    "colon_fence",
]
