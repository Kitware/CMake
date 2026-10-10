# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Generate the stylesheets that the Qt Help (.qch) rendition of the
# documentation uses, from `static-qthelp/qthelp.css.in` and the two palettes
# below.  The caller provides `qthelp_css_outdir`.
#
# Qt Creator serves a documentation set's own `offline-dark.css` in place of
# its `offline.css` or `offline-simple.css` when the Qt Creator theme is dark,
# so the pages link a stylesheet under one of those names and we ship a dark
# counterpart next to it.  Qt Creator 20 and older only substitute for
# `offline-simple.css`, and request `offline.css` instead when the theme is
# light, so the light palette is installed under both names.

# Light palette, matching `html_theme_options` in `conf.py.in` and the
# Pygments style in `colors.py`.
set(qthelp_body_bg          "#ffffff")
set(qthelp_body_fg          "#444444")
set(qthelp_link             "#2b63a8")
set(qthelp_link_visited     "#2b63a8")
set(qthelp_head_bg          "#f2f2f2")
set(qthelp_head_fg          "#003564")
set(qthelp_head_border      "#cccccc")
set(qthelp_headerlink       "#3d8ff2")
set(qthelp_muted            "#888888")
set(qthelp_pre_bg           "#eeeeee")
set(qthelp_pre_fg           "#333333")
set(qthelp_pre_border       "#aacc99")
set(qthelp_code_bg          "#ecf0f3")
set(qthelp_code_fg          "#333333")
set(qthelp_literal_link_bg  "#dfdfdf")
set(qthelp_topic_bg         "#eeeeee")
set(qthelp_note_bg          "#eeeeee")
set(qthelp_note_border      "#cccccc")
set(qthelp_seealso_bg       "#ffffcc")
set(qthelp_seealso_border   "#ffff66")
set(qthelp_warning_bg       "#ffe4e4")
set(qthelp_warning_border   "#ff6666")
set(qthelp_border           "#aaaaaa")
set(qthelp_hll_bg           "#ffffcc")
set(qthelp_tok_comment      "#408080")
set(qthelp_tok_operator     "#555555")
set(qthelp_tok_string       "#217a21")
set(qthelp_tok_number       "#105030")
set(qthelp_tok_builtin      "#333333")
set(qthelp_tok_function     "#007020")
set(qthelp_tok_variable     "#1080b0")
set(qthelp_tok_tag          "#bb60d5")
set(qthelp_tok_constant     "#4070a0")
set(qthelp_tok_entity       "#70a020")
set(qthelp_tok_attribute    "#906060")
set(qthelp_tok_label        "#a0a000")
set(qthelp_tok_exception    "#ff0000")
set(qthelp_tok_whitespace   "#bbbbbb")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/static-qthelp/qthelp.css.in"
  "${qthelp_css_outdir}/offline.css"
  @ONLY
  )
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/static-qthelp/qthelp.css.in"
  "${qthelp_css_outdir}/offline-simple.css"
  @ONLY
  )

# Dark palette.  The background and foreground match the stylesheet that
# Qt Creator falls back to for documentation that ships no dark variant, so
# that the CMake pages sit well next to the Qt pages in the same help pane.
set(qthelp_body_bg          "#262626")
set(qthelp_body_fg          "#d0d0d0")
set(qthelp_link             "#6ab0f3")
set(qthelp_link_visited     "#6ab0f3")
set(qthelp_head_bg          "#353637")
set(qthelp_head_fg          "#a8c8ea")
set(qthelp_head_border      "#4a4b4c")
set(qthelp_headerlink       "#6ab0f3")
set(qthelp_muted            "#9a9a9a")
set(qthelp_pre_bg           "#2d2e2f")
set(qthelp_pre_fg           "#d0d0d0")
set(qthelp_pre_border       "#4e5a46")
set(qthelp_code_bg          "#353637")
set(qthelp_code_fg          "#d0d0d0")
set(qthelp_literal_link_bg  "#3a3b3c")
set(qthelp_topic_bg         "#2d2e2f")
set(qthelp_note_bg          "#2d2e2f")
set(qthelp_note_border      "#4a4b4c")
set(qthelp_seealso_bg       "#39381f")
set(qthelp_seealso_border   "#5c5a30")
set(qthelp_warning_bg       "#3d2727")
set(qthelp_warning_border   "#7a4040")
set(qthelp_border           "#4a4b4c")
set(qthelp_hll_bg           "#4a4a2a")
set(qthelp_tok_comment      "#7fb7b7")
set(qthelp_tok_operator     "#b0b0b0")
set(qthelp_tok_string       "#6fc96f")
set(qthelp_tok_number       "#5fc090")
set(qthelp_tok_builtin      "#cccccc")
set(qthelp_tok_function     "#5fc45f")
set(qthelp_tok_variable     "#5fb8e8")
set(qthelp_tok_tag          "#d79ae8")
set(qthelp_tok_constant     "#85aee0")
set(qthelp_tok_entity       "#a8d060")
set(qthelp_tok_attribute    "#c89a9a")
set(qthelp_tok_label        "#cfcf60")
set(qthelp_tok_exception    "#ff6b6b")
set(qthelp_tok_whitespace   "#555555")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/static-qthelp/qthelp.css.in"
  "${qthelp_css_outdir}/offline-dark.css"
  @ONLY
  )
